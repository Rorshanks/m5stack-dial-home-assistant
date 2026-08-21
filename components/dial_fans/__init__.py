import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.components.sensor as sensor
import esphome.components.text_sensor as text_sensor
from esphome.const import CONF_ENTITY_ID, CONF_ID, CONF_NAME

CODEOWNERS = []
DEPENDENCIES = ["api", "sensor", "text_sensor"]

CONF_STATE_SENSOR = "state_sensor"
CONF_PERCENTAGE_SENSOR = "percentage_sensor"
CONF_DIRECTION_SENSOR = "direction_sensor"
CONF_LID_ENTITY_ID = "lid_entity_id"
CONF_LID_STATE_SENSOR = "lid_state_sensor"
CONF_LID_POSITION_SENSOR = "lid_position_sensor"

dial_fans_ns = cg.esphome_ns.namespace("dial_fans")
DialFans = dial_fans_ns.class_("DialFans", cg.Component)

FAN_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ENTITY_ID): cv.string,
        cv.Required(CONF_NAME): cv.string,
        cv.Optional(CONF_STATE_SENSOR): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_PERCENTAGE_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_DIRECTION_SENSOR): cv.use_id(text_sensor.TextSensor),
        # Optional MaxxFan-style vent lid, controlled as a separate `cover.` entity in HA.
        cv.Optional(CONF_LID_ENTITY_ID): cv.string,
        cv.Optional(CONF_LID_STATE_SENSOR): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_LID_POSITION_SENSOR): cv.use_id(sensor.Sensor),
    }
)


def _config_schema(value):
    fans = cv.All(cv.ensure_list(FAN_SCHEMA), cv.Length(min=0))(value)
    return {
        CONF_ID: cv.declare_id(DialFans)("dial_fans_id"),
        "fans": fans,
    }


CONFIG_SCHEMA = _config_schema


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    for fan in config["fans"]:
        entity = fan[CONF_ENTITY_ID]
        name = fan[CONF_NAME]
        # A bare Python None isn't a valid C++ codegen expression -- an
        # omitted optional sensor needs to render as an actual `nullptr`.
        state = cg.RawExpression("nullptr")
        percentage = cg.RawExpression("nullptr")
        direction = cg.RawExpression("nullptr")
        lid_entity_id = fan.get(CONF_LID_ENTITY_ID, "")
        lid_state = cg.RawExpression("nullptr")
        lid_position = cg.RawExpression("nullptr")
        if CONF_STATE_SENSOR in fan:
            state = await cg.get_variable(fan[CONF_STATE_SENSOR])
        if CONF_PERCENTAGE_SENSOR in fan:
            percentage = await cg.get_variable(fan[CONF_PERCENTAGE_SENSOR])
        if CONF_DIRECTION_SENSOR in fan:
            direction = await cg.get_variable(fan[CONF_DIRECTION_SENSOR])
        if CONF_LID_STATE_SENSOR in fan:
            lid_state = await cg.get_variable(fan[CONF_LID_STATE_SENSOR])
        if CONF_LID_POSITION_SENSOR in fan:
            lid_position = await cg.get_variable(fan[CONF_LID_POSITION_SENSOR])
        cg.add(
            var.add_fan(
                entity,
                name,
                state,
                percentage,
                direction,
                lid_entity_id,
                lid_state,
                lid_position,
            )
        )
