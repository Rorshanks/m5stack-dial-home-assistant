#include "dial_switches.h"

#include <cctype>

#include "esphome/core/log.h"

namespace esphome {
namespace dial_switches {

static const char *const TAG = "dial_switches";

namespace {
const std::string EMPTY_STRING;

bool is_on_state(const std::string &raw) {
  std::string lower = raw;
  for (char &c : lower)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return lower == "on" || lower == "true" || lower == "1";
}

bool is_valid_state(const std::string &raw) {
  if (raw.empty())
    return false;
  std::string lower = raw;
  for (char &c : lower)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return lower != "unknown" && lower != "unavailable" && lower != "none" && lower != "null";
}
}  // namespace

void DialSwitches::add_switch(const std::string &entity_id, const std::string &name, text_sensor::TextSensor *state) {
  for (const auto &existing : this->switches_) {
    if (existing.entity_id == entity_id) {
      ESP_LOGW(TAG, "Duplicate switch entity_id ignored: %s", entity_id.c_str());
      return;
    }
  }
  this->switches_.push_back({entity_id, name, state, false, false});
}

void DialSwitches::setup() {
  for (size_t i = 0; i < this->switches_.size(); i++) {
    auto &sw = this->switches_[i];
    if (sw.state != nullptr) {
      sw.state->add_on_state_callback([this, i](const std::string &value) { this->on_state_(i, value); });
      if (sw.state->has_state()) {
        this->on_state_(i, sw.state->state);
      }
    }
  }
}

void DialSwitches::on_state_(size_t index, const std::string &value) {
  auto &sw = this->switches_[index];
  if (is_valid_state(value)) {
    sw.state_valid = true;
    sw.is_on = is_on_state(value);
    return;
  }
  sw.state_valid = false;
}

const DialSwitches::SwitchEntry &DialSwitches::active_entry_() const {
  return this->switches_[this->active_index_];
}

bool DialSwitches::active_has_valid_state() const {
  if (this->switches_.empty() || this->active_index_ >= this->switches_.size())
    return false;
  return this->active_entry_().state_valid;
}

bool DialSwitches::active_is_on() const {
  if (this->switches_.empty() || this->active_index_ >= this->switches_.size())
    return false;
  return this->active_entry_().is_on;
}

void DialSwitches::select_switch(size_t index) {
  if (this->switches_.empty())
    return;
  this->active_index_ = index % this->switches_.size();
}

void DialSwitches::select_next() {
  if (!this->switches_.empty())
    this->active_index_ = (this->active_index_ + 1) % this->switches_.size();
}

void DialSwitches::select_previous() {
  if (!this->switches_.empty())
    this->active_index_ = (this->active_index_ + this->switches_.size() - 1) % this->switches_.size();
}

const std::string &DialSwitches::active_name() const { return this->name_at(this->active_index_); }

const std::string &DialSwitches::active_entity_id() const {
  if (this->switches_.empty())
    return EMPTY_STRING;
  return this->switches_[this->active_index_].entity_id;
}

const std::string &DialSwitches::name_at(size_t index) const {
  if (this->switches_.empty())
    return EMPTY_STRING;
  return this->switches_[index % this->switches_.size()].name;
}

SwitchCardSnapshot DialSwitches::card_snapshot_at(size_t index) const {
  SwitchCardSnapshot snap;
  if (this->switches_.empty() || index >= this->switches_.size())
    return snap;
  const auto &sw = this->switches_[index];
  snap.name = sw.name;
  snap.state_valid = sw.state_valid;
  snap.is_on = sw.is_on;
  return snap;
}

}  // namespace dial_switches
}  // namespace esphome
