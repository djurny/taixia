#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "taixia_switch.h"

namespace esphome {
namespace taixia {

static const char *const TAG = "taixia.switch";

  void TaiXiaSwitch::dump_config() {
    LOG_SWITCH("", "TaiXIA Switch", this);
    if (!this->parent_->have_sensors())
      this->parent_->send(6, 0, 0, SERVICE_ID_READ_STATUS, 0xffff);
  }

  void TaiXiaSwitch::write_state(bool state) {
    ASSERT(this->service_id_ < 0xff);
    bool publish = false;
    switch (this->sa_id_) {
      case SA_ID_CLIMATE:
        if (this->service_id_ == SERVICE_ID_CLIMATE_BEEPER) {
          state = !state;
        }
        publish = this->parent_->switch_command(this->sa_id_, this->service_id_, state);
        break;
      case SA_ID_DEHUMIDIFIER:
        if (this->service_id_ == SERVICE_ID_DEHUMIDTFIER_BEEPER) {
          state = !state;
        }
        publish = this->parent_->switch_command(this->sa_id_, this->service_id_, state);
        break;
    }
    if (publish) {
      this->publish_state(state);
    }
    this->parent_->read_appliance_status_conditional_();
  }

  void TaiXiaSwitch::handle_response(std::vector<uint8_t> &response) {
    uint8_t i;
    bool new_state = false;

    for (i = 3; i < response[0] - 3; i+=3) {
      ESP_LOGV(TAG, "handle_response[%d] {0x%2.2x, 0x%2.2x, 0x%2.2x}",
                    i, response[i+0], response[i+1], response[i+2]);

      if (this->service_id_ == response[i]) {
        new_state = bool(response[i + 2]);
        if (((this->sa_id_ == SA_ID_CLIMATE) && (this->service_id_ == SERVICE_ID_CLIMATE_BEEPER)) || 
            ((this->sa_id_ == SA_ID_DEHUMIDIFIER) && (this->service_id_ == SERVICE_ID_DEHUMIDTFIER_BEEPER))) {
            new_state = !new_state;
        }
        goto done;
      }
    }
    return;
done:
    if (this->state != new_state)
      this->publish_state(new_state);
  }

}  // namespace taixia
}  // namespace esphome
