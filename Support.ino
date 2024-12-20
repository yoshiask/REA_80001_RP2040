
void slottedLoop() {
  static uint32_t slot_100ms = 0;
  static uint32_t slot_10ms = 0;

  if (millis() >= slot_100ms) {
    Slot_100ms();
    while (millis() >= slot_100ms) {
      slot_100ms += 100;
    }
  }

  if (millis() >= slot_10ms) {
    Slot_10ms();
    while (millis() >= slot_10ms) {
      slot_10ms += 10;
    }
  }

  Slot_EveryLoop();
}

DeviceType deviceDetected = DEVICE_UNKNOWN;

float printAnalogPin(pin_size_t analogPin) {
  uint32_t analogAverage = 0;
  for (int i = 0; i < 10000; i++) {
    analogAverage += analogRead(analogPin);
  }
  return analogAverage / 10000.0;
}



void checkDeviceType() {
  analogReadResolution(12);
  Serial.print("CC1_UFP Voltage: ");
  Serial.println((3.3) * printAnalogPin(CC1_UFP) / (float)4095);
  Serial.print("CC1_DFP Voltage: ");
  Serial.println((3.3) * printAnalogPin(CC1_DFP) / (float)4095);
  Serial.print("CC1_DFP Voltage: ");
  Serial.println((3.3) * printAnalogPin(CC2_DFP) / (float)4095);

  // PinStatus deviceTypeRead = digitalRead(DEVICE_TYPE_PIN);
  // if (deviceTypeRead == LOW) {
  //   deviceDetected = DEVICE_STANDALONE_PSU;
  //   Serial.println("Device Detected: Standalone PSU");
  // }
  // if (deviceTypeRead == HIGH) {
  //   deviceDetected = DEVICE_ATTACHED_PSU;
  //   Serial.println("Device Detected: Attached PSU");
  // }
}

DeviceType getDeviceType() {
  return deviceDetected;
}

void printDeviceType() {
  checkDeviceType();
  if (deviceDetected == DEVICE_STANDALONE_PSU) {
    Serial.println("Device Detected: Standalone PSU");
  }
  if (deviceDetected == DEVICE_ATTACHED_PSU) {
    Serial.println("Device Detected: Attached PSU");
  }
}

void initializeStatusLED() {
  status_led.begin();
  status_led.clear();
  status_led.setPixelColor(LED_STATUS_ADDRESS, status_led.Color(20, 20, 20));
  //status_led.setPixelColor(LED_PD_STATUS_ADDRESS, status_led.Color(20, 0, 0));
  //while (!status_led.canShow()) {}
  status_led.show();
}

void updateStatusLED(PSUState commandedSupplyState) {
  if (commandedSupplyState == PSU_POWER_OFF) {
    status_led.setPixelColor(LED_STATUS_ADDRESS, status_led.Color(20, 20, 20));
  }
  if (commandedSupplyState == PSU_20V) {
    status_led.setPixelColor(LED_STATUS_ADDRESS, status_led.Color(0, 0, 20));
  }
  if (commandedSupplyState == PSU_12V) {
    status_led.setPixelColor(LED_STATUS_ADDRESS, status_led.Color(0, 20, 20));
  }
  if (commandedSupplyState == PSU_5V) {
    status_led.setPixelColor(LED_STATUS_ADDRESS, status_led.Color(0, 20, 0));
  }
}

static HUSB238_PDSelection pdLEDStatus;

void updatePDStatusLED(HUSB238_PDSelection pdStatus) {
  pdLEDStatus = pdStatus;
  if (pdStatus == PD_SRC_20V) {
    status_led.setPixelColor(LED_PD_STATUS_ADDRESS, status_led.Color(0, 20, 0));
  } else {
    status_led.setPixelColor(LED_PD_STATUS_ADDRESS, status_led.Color(20, 0, 0));
  }
}



void ledHandler(void) {
  status_led.clear();  
  updateStatusLED(getPSUStatus());
  updatePDStatusLED(pdLEDStatus);
  status_led.show();
}


void updatePowerState(PSUState commandedSupplyState) {
  Serial.print("Power Supply State: ");
  Serial.println(commandedSupplyState);
  powerStateMachineCommand(commandedSupplyState);
  psuState = commandedSupplyState;
}

void refreshStatusLED() {
  updateStatusLED(psuState);
}
