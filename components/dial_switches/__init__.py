import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.components.text_sensor as text_sensor
from esphome.const import CONF_ENTITY_ID, CONF_ID, CONF_NAME

CODEOWNERS = []
DEPENDENCIES = ["api", "text_sensor"]

CONF_STATE_SENSOR = "state_sensor"

dial_switches_ns = cg.esphome_ns.namespace("dial_switches")
DialSwitches = dial_switches_ns.class_("DialSwitches", cg.Component)

SWITCH_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ENTITY_ID): cv.string,
        cv.Required(CONF_NAME): cv.string,
        cv.Optional(CONF_STATE_SENSOR): cv.use_id(text_sensor.TextSensor),
    }
)


def _config_schema(value):
    switches = cv.All(cv.ensure_list(SWITCH_SCHEMA), cv.Length(min=0))(value)
    return {
        CONF_ID: cv.declare_id(DialSwitches)("dial_switches_id"),
        "switches": switches,
    }


CONFIG_SCHEMA = _config_schema


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    for sw in config["switches"]:
        entity = sw[CONF_ENTITY_ID]
        name = sw[CONF_NAME]
        state = None
        if CONF_STATE_SENSOR in sw:
            state = await cg.get_variable(sw[CONF_STATE_SENSOR])
        cg.add(var.add_switch(entity, name, state))
