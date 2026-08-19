#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace dial_fans {

enum class LidState : uint8_t {
  UNKNOWN = 0,
  OPEN = 1,
  CLOSED = 2,
  OPENING = 3,
  CLOSING = 4,
};

struct FanCardSnapshot {
  std::string name;
  bool state_valid{false};
  bool is_on{false};
  bool percent_valid{false};
  int percent{0};
  bool direction_valid{false};
  bool direction_reverse{false};
  bool has_lid{false};
  LidState lid_state{LidState::UNKNOWN};
  bool lid_position_valid{false};
  int lid_position_percent{0};
};

class DialFans : public Component {
 public:
  // lid_entity_id is a plain HA entity_id string (cover.xxx), not a sensor id -- it's used
  // directly by the page YAML when issuing cover.open_cover/close_cover/set_cover_position calls.
  void add_fan(const std::string &entity_id, const std::string &name, text_sensor::TextSensor *state = nullptr,
               sensor::Sensor *percentage = nullptr, text_sensor::TextSensor *direction = nullptr,
               const std::string &lid_entity_id = "", text_sensor::TextSensor *lid_state = nullptr,
               sensor::Sensor *lid_position = nullptr);
  void setup() override;

  size_t fan_count() const { return this->fans_.size(); }
  size_t active_index() const { return this->active_index_; }
  void select_fan(size_t index);
  void select_next();
  void select_previous();
  const std::string &active_name() const;
  const std::string &active_entity_id() const;
  const std::string &active_lid_entity_id() const;
  const std::string &name_at(size_t index) const;
  FanCardSnapshot card_snapshot_at(size_t index) const;

  bool active_has_valid_state() const;
  bool active_is_on() const;
  bool active_percent_valid() const;
  int active_percent() const;
  bool active_direction_valid() const;
  bool active_direction_reverse() const;
  bool active_has_lid() const;
  LidState active_lid_state() const;
  bool active_lid_position_valid() const;
  int active_lid_position_percent() const;

 protected:
  struct FanEntry {
    std::string entity_id;
    std::string name;
    text_sensor::TextSensor *state{nullptr};
    bool state_valid{false};
    bool is_on{false};
    sensor::Sensor *percentage{nullptr};
    bool percent_valid{false};
    int percent{0};
    text_sensor::TextSensor *direction{nullptr};
    bool direction_valid{false};
    bool direction_reverse{false};
    std::string lid_entity_id;
    text_sensor::TextSensor *lid_state{nullptr};
    LidState lid_state_value{LidState::UNKNOWN};
    sensor::Sensor *lid_position{nullptr};
    bool lid_position_valid{false};
    int lid_position_percent{0};
  };

  const FanEntry &active_entry_() const;
  void on_state_(size_t index, const std::string &value);
  void on_percentage_(size_t index, float value);
  void on_direction_(size_t index, const std::string &value);
  void on_lid_state_(size_t index, const std::string &value);
  void on_lid_position_(size_t index, float value);

  std::vector<FanEntry> fans_;
  size_t active_index_{0};
};

}  // namespace dial_fans
}  // namespace esphome
