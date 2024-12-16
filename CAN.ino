

void my_cb(struct can2040* cd, uint32_t notify, struct can2040_msg* msg) {

  (void)(cd);

  switch (notify) {
    case CAN2040_NOTIFY_RX:
      //Serial.printf("cb: message received\n");
      rx_msg = *msg;
      got_msg = true;
      break;
    case CAN2040_NOTIFY_TX:
      //Serial.printf("cb: message sent ok\n");
      break;
    case CAN2040_NOTIFY_ERROR:
      Serial.printf("cb: an error occurred\n");
      break;
    default:
      Serial.printf("cb: unknown event type\n");
      break;
  }
}

void checkCANMessages() {
  if (got_msg) {
    got_msg = false;
    uint8_t ident = rx_msg.id;
    uint8_t CANMessageType = rx_msg.data[0];
    uint8_t parameter = rx_msg.data[1];
    // Serial.println(ident);
    // Serial.println(CANMessageType);
    // Serial.println(parameter);
    if (ident == CAN_IDENTIFIER) {
      if (CANMessageType == CAN_PSU_VOLTAGE) {
        if (parameter > 0 && parameter <= PSU_5V) {
          updatePowerState((PSUState)parameter);
        }
      }
      if (getDeviceType() == DEVICE_ATTACHED_PSU) {
        if (CANMessageType == CAN_OUTPUT_POLARITY) {
          reportOutCurrent();
        }        
        if (CANMessageType == CAN_CURRENT_REQUEST) {
          reportOutCurrent();
        }
        if (CANMessageType == CAN_CURRENT_ZERO_REQUEST) {
          zeroCurrentSense();
        }
      }
    }
  }
}