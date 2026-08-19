#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace dial_switches {

struct SwitchCardSnapshot {
  std::string name;
  bool state_valid{false};
  bool is_on{false};
};

class DialSwitches : public Component {
 public:
  void add_switch(const std::string &entity_id, const std::string &name, text_sensor::TextSensor *state = nullptr);
  void setup() override;

  size_t switch_count() const { return this->switches_.size(); }
  size_t active_index() const { return this->active_index_; }
  void select_switch(size_t index);
  void select_next();
  void select_previous();
  const std::string &active_name() const;
  const std::string &active_entity_id() const;
  const std::string &name_at(size_t index) const;
  SwitchCardSnapshot card_snapshot_at(size_t index) const;

  bool active_has_valid_state() const;
  bool active_is_on() const;

 protected:
  struct SwitchEntry {
    std::string entity_id;
    std::string name;
    text_sensor::TextSensor *state{nullptr};
    bool state_valid{false};
    bool is_on{false};
  };

  const SwitchEntry &active_entry_() const;
  void on_state_(size_t index, const std::string &value);

  std::vector<SwitchEntry> switches_;
  size_t active_index_{0};
};

}  // namespace dial_switches
}  // namespace esphome
