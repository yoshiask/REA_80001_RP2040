void printCableDFP(void) {
  Serial.print("CC1 DFP: ");
  Serial.println(averageAnalogPinA0());
  Serial.print("CC2 DFP: ");
  Serial.println(averageAnalogPinA1());
}


float averageAnalogPinA0(void) {
  uint32_t analogAverage = 0;
  for (int i = 0; i < 10000; i++) {
    analogAverage += analogRead(A0);
  }
  return analogAverage / 10000.0;
}

float averageAnalogPinA1(void) {
  uint32_t analogAverage = 0;
  for (int i = 0; i < 10000; i++) {
    analogAverage += analogRead(A1);
  }
  return analogAverage / 10000.0;
}
