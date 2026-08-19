#include "dial_fans.h"

#include <cctype>
#include <cmath>

#include "esphome/core/log.h"

namespace esphome {
namespace dial_fans {

static const char *const TAG = "dial_fans";

namespace {
const std::string EMPTY_STRING;

std::string to_lower(const std::string &raw) {
  std::string lower = raw;
  for (char &c : lower)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return lower;
}

bool is_valid_state(const std::string &raw) {
  if (raw.empty())
    return false;
  const std::string lower = to_lower(raw);
  return lower != "unknown" && lower != "unavailable" && lower != "none" && lower != "null";
}

bool is_on_state(const std::string &raw) {
  const std::string lower = to_lower(raw);
  return lower == "on" || lower == "true" || lower == "1";
}

bool parse_percent(float raw, int &percent) {
  if (!std::isfinite(raw))
    return false;
  float clamped = raw;
  if (clamped < 0.0f)
    clamped = 0.0f;
  else if (clamped > 100.0f)
    clamped = 100.0f;
  percent = static_cast<int>(clamped + 0.5f);
  return true;
}

LidState parse_lid_state(const std::string &raw) {
  const std::string lower = to_lower(raw);
  if (lower == "open")
    return LidState::OPEN;
  if (lower == "closed")
    return LidState::CLOSED;
  if (lower == "opening")
    return LidState::OPENING;
  if (lower == "closing")
    return LidState::CLOSING;
  return LidState::UNKNOWN;
}
}  // namespace

void DialFans::add_fan(const std::string &entity_id, const std::string &name, text_sensor::TextSensor *state,
                       sensor::Sensor *percentage, text_sensor::TextSensor *direction,
                       const std::string &lid_entity_id, text_sensor::TextSensor *lid_state,
                       sensor::Sensor *lid_position) {
  for (const auto &existing : this->fans_) {
    if (existing.entity_id == entity_id) {
      ESP_LOGW(TAG, "Duplicate fan entity_id ignored: %s", entity_id.c_str());
      return;
    }
  }
  FanEntry entry;
  entry.entity_id = entity_id;
  entry.name = name;
  entry.state = state;
  entry.percentage = percentage;
  entry.direction = direction;
  entry.lid_entity_id = lid_entity_id;
  entry.lid_state = lid_state;
  entry.lid_position = lid_position;
  this->fans_.push_back(entry);
}

void DialFans::setup() {
  for (size_t i = 0; i < this->fans_.size(); i++) {
    auto &fan = this->fans_[i];
    if (fan.state != nullptr) {
      fan.state->add_on_state_callback([this, i](const std::string &value) { this->on_state_(i, value); });
      if (fan.state->has_state())
        this->on_state_(i, fan.state->state);
    }
    if (fan.percentage != nullptr) {
      fan.percentage->add_on_state_callback([this, i](float value) { this->on_percentage_(i, value); });
      if (fan.percentage->has_state())
        this->on_percentage_(i, fan.percentage->state);
    }
    if (fan.direction != nullptr) {
      fan.direction->add_on_state_callback([this, i](const std::string &value) { this->on_direction_(i, value); });
      if (fan.direction->has_state())
        this->on_direction_(i, fan.direction->state);
    }
    if (fan.lid_state != nullptr) {
      fan.lid_state->add_on_state_callback([this, i](const std::string &value) { this->on_lid_state_(i, value); });
      if (fan.lid_state->has_state())
        this->on_lid_state_(i, fan.lid_state->state);
    }
    if (fan.lid_position != nullptr) {
      fan.lid_position->add_on_state_callback([this, i](float value) { this->on_lid_position_(i, value); });
      if (fan.lid_position->has_state())
        this->on_lid_position_(i, fan.lid_position->state);
    }
  }
}

void DialFans::on_state_(size_t index, const std::string &value) {
  auto &fan = this->fans_[index];
  if (is_valid_state(value)) {
    fan.state_valid = true;
    fan.is_on = is_on_state(value);
    return;
  }
  fan.state_valid = false;
}

void DialFans::on_percentage_(size_t index, float value) {
  auto &fan = this->fans_[index];
  int percent = 0;
  if (parse_percent(value, percent)) {
    fan.percent_valid = true;
    fan.percent = percent;
  }
}

void DialFans::on_direction_(size_t index, const std::string &value) {
  auto &fan = this->fans_[index];
  if (!is_valid_state(value)) {
    fan.direction_valid = false;
    return;
  }
  fan.direction_valid = true;
  fan.direction_reverse = (to_lower(value) == "reverse");
}

void DialFans::on_lid_state_(size_t index, const std::string &value) {
  auto &fan = this->fans_[index];
  fan.lid_state_value = is_valid_state(value) ? parse_lid_state(value) : LidState::UNKNOWN;
}

void DialFans::on_lid_position_(size_t index, float value) {
  auto &fan = this->fans_[index];
  int percent = 0;
  if (parse_percent(value, percent)) {
    fan.lid_position_valid = true;
    fan.lid_position_percent = percent;
  }
}

const DialFans::FanEntry &DialFans::active_entry_() const { return this->fans_[this->active_index_]; }

bool DialFans::active_has_valid_state() const {
  if (this->fans_.empty() || this->active_index_ >= this->fans_.size())
    return false;
  return this->active_entry_().state_valid;
}

bool DialFans::active_is_on() const {
  if (this->fans_.empty() || this->active_index_ >= this->fans_.size())
    return false;
  return this->active_entry_().is_on;
}

bool DialFans::active_percent_valid() const {
  if (this->fans_.empty() || this->active_index_ >= this->fans_.size())
    return false;
  return this->active_entry_().percent_valid;
}

int DialFans::active_percent() const {
  if (this->fans_.empty() || this->active_index_ >= this->fans_.size())
    return 0;
  return this->active_entry_().percent;
}

bool DialFans::active_direction_valid() const {
  if (this->fans_.empty() || this->active_index_ >= this->fans_.size())
    return false;
  return this->active_entry_().direction_valid;
}

bool DialFans::active_direction_reverse() const {
  if (this->fans_.empty() || this->active_index_ >= this->fans_.size())
    return false;
  return this->active_entry_().direction_reverse;
}

bool DialFans::active_has_lid() const {
  if (this->fans_.empty() || this->active_index_ >= this->fans_.size())
    return false;
  return !this->active_entry_().lid_entity_id.empty();
}

LidState DialFans::active_lid_state() const {
  if (this->fans_.empty() || this->active_index_ >= this->fans_.size())
    return LidState::UNKNOWN;
  return this->active_entry_().lid_state_value;
}

bool DialFans::active_lid_position_valid() const {
  if (this->fans_.empty() || this->active_index_ >= this->fans_.size())
    return false;
  return this->active_entry_().lid_position_valid;
}

int DialFans::active_lid_position_percent() const {
  if (this->fans_.empty() || this->active_index_ >= this->fans_.size())
    return 0;
  return this->active_entry_().lid_position_percent;
}

void DialFans::select_fan(size_t index) {
  if (this->fans_.empty())
    return;
  this->active_index_ = index % this->fans_.size();
}

void DialFans::select_next() {
  if (!this->fans_.empty())
    this->active_index_ = (this->active_index_ + 1) % this->fans_.size();
}

void DialFans::select_previous() {
  if (!this->fans_.empty())
    this->active_index_ = (this->active_index_ + this->fans_.size() - 1) % this->fans_.size();
}

const std::string &DialFans::active_name() const { return this->name_at(this->active_index_); }

const std::string &DialFans::active_entity_id() const {
  if (this->fans_.empty())
    return EMPTY_STRING;
  return this->fans_[this->active_index_].entity_id;
}

const std::string &DialFans::active_lid_entity_id() const {
  if (this->fans_.empty())
    return EMPTY_STRING;
  return this->fans_[this->active_index_].lid_entity_id;
}

const std::string &DialFans::name_at(size_t index) const {
  if (this->fans_.empty())
    return EMPTY_STRING;
  return this->fans_[index % this->fans_.size()].name;
}

FanCardSnapshot DialFans::card_snapshot_at(size_t index) const {
  FanCardSnapshot snap;
  if (this->fans_.empty() || index >= this->fans_.size())
    return snap;
  const auto &fan = this->fans_[index];
  snap.name = fan.name;
  snap.state_valid = fan.state_valid;
  snap.is_on = fan.is_on;
  snap.percent_valid = fan.percent_valid;
  snap.percent = fan.percent;
  snap.direction_valid = fan.direction_valid;
  snap.direction_reverse = fan.direction_reverse;
  snap.has_lid = !fan.lid_entity_id.empty();
  snap.lid_state = fan.lid_state_value;
  snap.lid_position_valid = fan.lid_position_valid;
  snap.lid_position_percent = fan.lid_position_percent;
  return snap;
}

}  // namespace dial_fans
}  // namespace esphome
