#define PSU_I2C_ADDRESS 0x56

FullBridgeType fullBridgeState = FULL_BRIDGE_OFF;
FullBridgeType previousState = FULL_BRIDGE_OFF;
FullBridgeType desiredState = FULL_BRIDGE_OFF;

static FullBridgePolarity fullBridgePolarity = FULL_BRIDGE_POLARITY_POSITIVE;

// Initialization for the pins
void initializeFullBridge() {
  fullBridgeOff();
  pinMode(FB_B_H, OUTPUT);
  pinMode(FB_B_L, OUTPUT);
  pinMode(FB_A_H, OUTPUT);
  pinMode(FB_A_L, OUTPUT);
}

// Turn off all connections
void fullBridgeOff() {
  digitalWrite(FB_B_H, LOW);
  digitalWrite(FB_B_L, HIGH);
  digitalWrite(FB_A_H, LOW);
  digitalWrite(FB_A_L, HIGH);
}

// Enable positive connection
void fullBridgePositive() {
  digitalWrite(FB_B_H, LOW);
  digitalWrite(FB_B_L, HIGH);
  digitalWrite(FB_A_H, HIGH);
  digitalWrite(FB_A_L, LOW);
}

// Enable negative connection
void fullBridgeNegative() {
  digitalWrite(FB_B_H, HIGH);
  digitalWrite(FB_B_L, LOW);
  digitalWrite(FB_A_H, LOW);
  digitalWrite(FB_A_L, HIGH);
}

int offCycles = 0;  // Tracks OFF cycles when transitioning from positive to negative or vice versa

// State machine for the full bridge
void fullBridgeStateMachine() {
  // Handle transitions and on-enter conditions
  switch (fullBridgeState) {
    case FULL_BRIDGE_OFF:
      if (fullBridgeState != previousState) {
        Serial.println("FULL_BRIDGE_OFF");
        fullBridgeOff();
        offCycles = 0;
        previousState = fullBridgeState;
      }

      if (desiredState == FULL_BRIDGE_POSITIVE || desiredState == FULL_BRIDGE_NEGATIVE) {
        offCycles++;
        if (offCycles >= 2) {
          fullBridgeState = desiredState;
          offCycles = 0;
        }
      }
      break;

    case FULL_BRIDGE_POSITIVE:
      if (fullBridgeState != previousState) {
        Serial.println("FULL_BRIDGE_POSITIVE");
        fullBridgePositive();
        previousState = fullBridgeState;
      }

      if (desiredState == FULL_BRIDGE_NEGATIVE) {
        fullBridgeState = FULL_BRIDGE_OFF;
      } else if (desiredState == FULL_BRIDGE_OFF) {
        fullBridgeState = FULL_BRIDGE_OFF;
      }
      break;

    case FULL_BRIDGE_NEGATIVE:
      if (fullBridgeState != previousState) {
        Serial.println("FULL_BRIDGE_NEGATIVE");
        fullBridgeNegative();
        previousState = fullBridgeState;
      }
      if (desiredState == FULL_BRIDGE_POSITIVE) {
        fullBridgeState = FULL_BRIDGE_OFF;
      } else if (desiredState == FULL_BRIDGE_OFF) {
        fullBridgeState = FULL_BRIDGE_OFF;
      }
      break;

    default:
      if (fullBridgeState != previousState) {
        Serial.println("Entering unknown state, defaulting to FULL_BRIDGE_OFF");
        fullBridgeOff();
        previousState = fullBridgeState;
      }
      fullBridgeState = FULL_BRIDGE_OFF;
      break;
  }
}

// Example method to set the desired state
void setDesiredFullBridgeState(FullBridgeType newDesiredState) {
  desiredState = newDesiredState;
}

void turnOnFullBridge() {
  if (fullBridgePolarity == FULL_BRIDGE_POLARITY_POSITIVE) {
    setDesiredFullBridgeState(FULL_BRIDGE_POSITIVE);
  }
  // if (fullBridgePolarity == FULL_BRIDGE_POLARITY_NEGATIVE) {
  //   setDesiredFullBridgeState(FULL_BRIDGE_NEGATIVE);
  // }
}

void turnOffFullBridge() {
  setDesiredFullBridgeState(FULL_BRIDGE_OFF);
}

void setFullBridgePolarity(FullBridgePolarity polarity) {
  if (polarity == FULL_BRIDGE_POLARITY_POSITIVE) {
    fullBridgePolarity = polarity;
    Serial.println("CAN Received Full Bridge Positive");
  }
  if (polarity == FULL_BRIDGE_POLARITY_NEGATIVE) {
    //fullBridgePolarity = polarity;
    Serial.println("CAN Received Full Bridge Negative, Command Disabled.");
  }
}

void initalizeFullBridge() {
  pinMode(FB_B_H, OUTPUT);
  pinMode(FB_B_L, OUTPUT);
  pinMode(FB_A_H, OUTPUT);
  pinMode(FB_A_L, OUTPUT);
  digitalWrite(PSU_CONNECT_OUTPUT_PIN, LOW);
  digitalWrite(CONNECT_INPUT_PIN, LOW);
  digitalWrite(PSU_STANDBY_PIN, HIGH);
  digitalWrite(PSU_EN_12V_PIN, LOW);
}

void initializePSUPins() {
  pinMode(PSU_CONNECT_OUTPUT_PIN, OUTPUT);
  pinMode(PSU_STANDBY_PIN, OUTPUT);
  pinMode(PSU_EN_12V_PIN, OUTPUT);
  pinMode(CONNECT_INPUT_PIN, OUTPUT);
  digitalWrite(PSU_CONNECT_OUTPUT_PIN, LOW);
  digitalWrite(CONNECT_INPUT_PIN, LOW);
  digitalWrite(PSU_STANDBY_PIN, HIGH);
  digitalWrite(PSU_EN_12V_PIN, LOW);
}

PSUState getPSUStatus(void) {
  return psuState;
}

void powerStateMachineCommand(PSUState commandedState) {
  psuState = commandedState;

  digitalWrite(PSU_CONNECT_OUTPUT_PIN, LOW);
  digitalWrite(CONNECT_INPUT_PIN, LOW);
  delay(100);

  if (psuState == PSU_POWER_OFF) {
    digitalWrite(PSU_STANDBY_PIN, HIGH);
    digitalWrite(PSU_EN_12V_PIN, LOW);
    turnOffFullBridge();
    updateStatusLED(psuState);
    Serial.println("LED Power Off");
    sendPSUStatusCommand(psuState, PSU_OK);
  }
  /*
  if (psuState == PSU_20V) {
    digitalWrite(PSU_STANDBY_PIN, HIGH);
    digitalWrite(PSU_EN_12V_PIN, LOW);
    digitalWrite(CONNECT_INPUT_PIN, HIGH);
    updateStatusLED(psuState);
    Serial.println("LED Power 20V Enabled");
  }
  */
  if (psuState == PSU_12V) {
    digitalWrite(PSU_STANDBY_PIN, LOW);
    digitalWrite(PSU_EN_12V_PIN, HIGH);
    turnOnFullBridge();
    delay(2);
    digitalWrite(PSU_CONNECT_OUTPUT_PIN, HIGH);
    updateStatusLED(psuState);
    Serial.println("LED Power 12V Enabled");
    sendPSUStatusCommand(psuState, PSU_OK);
  }
  if (psuState == PSU_5V) {
    digitalWrite(PSU_STANDBY_PIN, LOW);
    digitalWrite(PSU_EN_12V_PIN, LOW);
    turnOnFullBridge();
    delay(2);
    digitalWrite(PSU_CONNECT_OUTPUT_PIN, HIGH);
    updateStatusLED(psuState);
    Serial.println("LED Power 5V Enabled");
    sendPSUStatusCommand(psuState, PSU_OK);
  }
}

void sendPSUStatusCommand(PSUState state, PSUStatus status) {
  Serial.println("Sending PSU Status Message");
  tx_msg.id = CAN_IDENTIFIER;
  tx_msg.dlc = 8;
  tx_msg.data[0] = CAN_PSU_STATUS;
  tx_msg.data[1] = state;
  tx_msg.data[2] = status;
  tx_msg.data[3] = CAN_STUFFING_FRAME;
  tx_msg.data[4] = CAN_STUFFING_FRAME;
  tx_msg.data[5] = CAN_STUFFING_FRAME;
  tx_msg.data[6] = CAN_STUFFING_FRAME;
  tx_msg.data[7] = CAN_STUFFING_FRAME;
  can2040.send_message(&tx_msg);
}

void readAndDisplayPSU() {
  int address = PSU_I2C_ADDRESS;
  Serial.println("Reading PSU registers...");

  for (uint8_t reg = 0; reg < 254; reg++) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    uint8_t status = Wire.endTransmission();

    if (status == 0) {
      Wire.requestFrom(address, 1);
      if (Wire.available()) {
        uint8_t value = Wire.read();
        if (value != 0xFF) {
          Serial.print("Register 0x");
          Serial.print(reg, HEX);
          Serial.print(": 0x");
          Serial.println(value, HEX);
        }
      }
    }
  }

  Serial.println("Done reading PSU registers.");
}

void readInputVoltage() {
  int address = PSU_I2C_ADDRESS;
  Wire.beginTransmission(address);
  Wire.write(0x88);
  uint8_t status = Wire.endTransmission();

  if (status == 0) {
    Wire.requestFrom(address, 2);
    uint8_t HIGH = 0;
    uint8_t LOW = 0;
    uint8_t byteNum = 0;
    while (Wire.available()) {
      uint8_t value = Wire.read();
      if (byteNum == 1) {
        HIGH = value;
      }
      if (byteNum == 0) {
        LOW = value;
      }
      byteNum++;
    }
    float voltage = ((float)((HIGH << 8) | LOW)) / 92.5;
    Serial.print("Input Voltage Value: ");
    Serial.print(voltage);
    Serial.println("V");
  }
}