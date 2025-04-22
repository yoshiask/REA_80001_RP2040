#define VOLTS_PER_AMP 0.5

const int currentSensePin = CURRENT_SENSE;  // Analog input pin that the potentiometer is attached to

float currentSenseZero = 0;

bool currentReportRequest = false;

void initCurrentSense() {
  analogReadResolution(12);
  currentSenseZero = readCurrentSenseVoltage();
}

void zeroCurrentSense() {
  Serial.println("Zeroing Current Sense");
  currentSenseZero = readCurrentSenseVoltage();
  reportOutCurrent();
}

void reportOutCurrent() {
  float currentValue = readCurrentSenseCurrent();
  uint16_t currentSend = 0;
  if (currentValue > 0) {
    currentSend = (uint16_t)(currentValue * 1000);
  }

  uint8_t highByte = (currentSend >> 8) & 0xFF;  // Get the high byte
  uint8_t lowByte = currentSend & 0xFF;          // Get the low byte

  sendCurrentDataCommand(highByte, lowByte);

  Serial.print("Current Report Request Fufilled.");
  Serial.print("Current Values sent: ");
  Serial.print(currentValue, 3);
  Serial.print(" - ");
  Serial.print(currentSend);
  Serial.print(" - ");
  Serial.print(highByte);
  Serial.print(" - ");
  Serial.print(highByte);
  Serial.println(" - ");
}

void sendCurrentDataCommand(uint8_t byte_high, uint8_t byte_low) {
  Serial.println("Sending Current Data");
  tx_msg.id = CAN_IDENTIFIER;
  tx_msg.dlc = 8;
  tx_msg.data[0] = CAN_CURRENT_DATA;
  tx_msg.data[1] = byte_high;
  tx_msg.data[2] = byte_low;
  tx_msg.data[3] = CAN_STUFFING_FRAME;
  tx_msg.data[4] = CAN_STUFFING_FRAME;
  tx_msg.data[5] = CAN_STUFFING_FRAME;
  tx_msg.data[6] = CAN_STUFFING_FRAME;
  tx_msg.data[7] = CAN_STUFFING_FRAME;
  can2040.send_message(&tx_msg);
}

void printCurrentSenseVoltage() {
  uint32_t startTime = millis();
  Serial.print("Counts: ");
  Serial.println(analogRead(currentSensePin));
  Serial.print("Voltage: ");
  Serial.println(readCurrentSenseVoltage(), 4);
  Serial.print("Zeroed Voltage: ");
  Serial.println(readCurrentSenseVoltage() - currentSenseZero, 4);
  float sensedCurrent = readCurrentSenseCurrent();
  Serial.print("Current: ");
  Serial.println(sensedCurrent, 4);
  Serial.print("Time taken (ms):");
  Serial.println(millis() - startTime);
}

void printCurrentContinuously() {
  uint32_t startTime = millis();
  while (startTime + 30000 > millis()) {
    float sensedCurrent = readCurrentSenseCurrent();
    Serial.println(sensedCurrent, 4);
  }
  Serial.println({ "30 Seconds has elapsed." });
}

float readCurrentSenseCurrent() {
  return (readCurrentSenseVoltage() - currentSenseZero) / VOLTS_PER_AMP;
}

float readCurrentSenseVoltage() {
  uint32_t currentSenseAverage = 0;
  for (int i = 0; i < 10000; i++) {
    currentSenseAverage += analogRead(currentSensePin);
  }
  float currentSenseValue = currentSenseAverage / 10000.0;
  float currentSenseVoltage = (3.3) * (float)currentSenseValue / (float)4095;
  currentSenseVoltage = (5 / 3.3) * currentSenseVoltage;
  return currentSenseVoltage;
}

float readCurrentSenseCurrentFast() {
  return (readCurrentSenseVoltageFast() - currentSenseZero) / VOLTS_PER_AMP;
}

float readCurrentSenseVoltageFast() {
  uint32_t currentSenseAverage = 0;
  for (int i = 0; i < 100; i++) {
    currentSenseAverage += analogRead(currentSensePin);
  }
  float currentSenseValue = currentSenseAverage / 100.0;
  float currentSenseVoltage = (3.3) * (float)currentSenseValue / (float)4095;
  currentSenseVoltage = (5 / 3.3) * currentSenseVoltage;
  return currentSenseVoltage;
}