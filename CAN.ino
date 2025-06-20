

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

void terminateCAN() {
  pinMode(CAN_NTERM, OUTPUT);
  digitalWrite(CAN_NTERM, LOW);
}

static bool printCAN = false;

void toggleCANPrinting() {
  printCAN = !printCAN;
  if(printCAN){
    Serial.println("Printing received CAN messages: Enabled");
  } else {
    Serial.println("Printing received CAN messages: Disabled");
  }
}

void printCANMessage(uint8_t ident, uint8_t CANMessageType, uint8_t parameter1, uint8_t parameter2) {
  if (printCAN) {
    Serial.print("CAN: ");
    Serial.print(ident);
    Serial.print(" , ");
    Serial.print(CANMessageType);
    Serial.print(" , ");
    Serial.print(parameter1);
    Serial.print(" , ");
    Serial.print(parameter2);
  }
}

void checkCANMessages() {
  if (got_msg) {
    got_msg = false;
    uint8_t ident = rx_msg.id;
    uint8_t CANMessageType = rx_msg.data[0];
    uint8_t parameter1 = rx_msg.data[1];
    uint8_t parameter2 = rx_msg.data[2];
    printCANMessage(ident, CANMessageType, parameter1, parameter2);
    if (ident == CAN_IDENTIFIER) {
      if (CANMessageType == CAN_PSU_VOLTAGE) {
        if (parameter1 > 0 && parameter1 <= PSU_5V) {
          updatePowerState((PSUState)parameter1);
        }
      }
      if (CANMessageType == CAN_OUTPUT_POLARITY) {
        reportOutCurrent();
      }
      if (CANMessageType == CAN_CURRENT_REQUEST) {
        reportOutCurrent();
      }
      if (CANMessageType == CAN_CURRENT_ZERO_REQUEST) {
        zeroCurrentSense();
      }
      if (CANMessageType == CAN_RUN_POLARITY_CHECK) {
        startPolarityDetect((PSUState)parameter1, (PolarityDetectType)parameter2);
      }
      if (CANMessageType == CAN_SET_FULL_BRIDGE) {
        setDesiredFullBridgeState((FullBridgeType)parameter1);
      }     
    }
  }
}

void pingCAN() {
  tx_msg.id = CAN_IDENTIFIER;
  tx_msg.dlc = 8;
  tx_msg.data[0] = CAN_PING;
  tx_msg.data[1] = CAN_STUFFING_FRAME;
  tx_msg.data[2] = CAN_STUFFING_FRAME;
  tx_msg.data[3] = CAN_STUFFING_FRAME;
  tx_msg.data[4] = CAN_STUFFING_FRAME;
  tx_msg.data[5] = CAN_STUFFING_FRAME;
  tx_msg.data[6] = CAN_STUFFING_FRAME;
  tx_msg.data[7] = CAN_STUFFING_FRAME;
  can2040.send_message(&tx_msg);
}
