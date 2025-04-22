bool currentDetected = false;
bool rampEnabled = false;
bool forwardRamp = false;
uint16_t rampValue = 0;
bool forwardShort = false;
bool reverseShort = false;
bool runPolarityDetection = false;
uint16_t rampDelay = 0;
PolarityDetectType polarityDetectStatus = POLARITY_DETECT_NOT_RUN;
PolarityDetectState polarityCurrentState = POL_IDLE;
PolarityDetectState polarityPreviousState = POL_IDLE;

void startRamp() {
  Serial.println("Starting Ramp");
  analogWriteFreq(10000);
  analogWriteRange(2000);
  rampValue = 0;
  rampEnabled = true;
}

void startPolarityDetect() {
  runPolarityDetection = true;
  setDesiredFullBridgeState(FULL_BRIDGE_OFF);
  setPSUState(PSU_POWER_OFF);
    analogWriteFreq(10000);
  analogWriteRange(10000);
}

PolarityDetectType getPolarityDetectStatus(){
  return polarityDetectStatus;
}

void polarityDetectHandler() {
  static bool firstRunInState = true;

  // Detect state change
  if (polarityCurrentState != polarityPreviousState) {
    firstRunInState = true;
    polarityPreviousState = polarityCurrentState;
  }

  switch (polarityCurrentState) {
    case POL_IDLE:
      if (firstRunInState) {
        firstRunInState = false;
      }

      if (runPolarityDetection && getFullBridgeState() == FULL_BRIDGE_OFF && getPSUStatus() == PSU_POWER_OFF) {
        polarityCurrentState = POL_CHECK_FORWARD;
        runPolarityDetection = false;
        polarityDetectStatus = POLARITY_DETECT_NOT_RUN;
      }
      break;

    case POL_CHECK_FORWARD:
      if (firstRunInState) {
        if (verboseLevel >= 1) Serial.println("Entering CHECK_FORWARD_POLARITY");
        firstRunInState = false;
        rampEnabled = true;
        forwardRamp = true;
        //setDesiredFullBridgeState(FULL_BRIDGE_POSITIVE);
        digitalWrite(FB_B_H, LOW);
        digitalWrite(FB_B_L, HIGH);
        digitalWrite(FB_A_H, LOW);
        digitalWrite(FB_A_L, LOW);
        setPSUState(PSU_5V);
      }

      if (rampEnabled == false) {
        if (currentDetected) {
          forwardShort = true;
        } else {
          forwardShort = false;
        }
        polarityCurrentState = POL_CHECK_REVERSE;
      }
      break;

    case POL_CHECK_REVERSE:
      if (firstRunInState) {
        if (verboseLevel >= 1) Serial.println("Entering CHECK_REVERSE_POLARITY");
        firstRunInState = false;
        rampEnabled = true;
        forwardRamp = false;
        //setDesiredFullBridgeState(FULL_BRIDGE_NEGATIVE);
        digitalWrite(FB_B_H, LOW);
        digitalWrite(FB_B_L, LOW);
        digitalWrite(FB_A_H, LOW);
        digitalWrite(FB_A_L, HIGH);
      }

      if (rampEnabled == false) {
        if (currentDetected) {
          reverseShort = true;
        } else {
          reverseShort = false;
        }
        polarityCurrentState = POL_IDLE;
        setPSUState(PSU_POWER_OFF);
        setDesiredFullBridgeState(FULL_BRIDGE_OFF);
        fullBridgeOff();
        if (!forwardShort && !reverseShort) {
          Serial.println("No strip detected");
          polarityDetectStatus = POLARITY_NO_DETECT;
        }
        if (forwardShort && !reverseShort) {
          Serial.println("Strip detected with reverse polarity");
          polarityDetectStatus = POLARITY_REVERSE;
        }
        if (!forwardShort && reverseShort) {
          Serial.println("Strip detected with forward polarity");
          polarityDetectStatus = POLARITY_FORWARD;
        }
        if (forwardShort && reverseShort) {
          Serial.println("Output Shorted");
          polarityDetectStatus = POLARITY_SHORTED;
        }
        sendPolarityStatusCommand(polarityDetectStatus);
      }
      break;
  }
}

void rampHandler() {
  if (rampEnabled && getPSUStatus() == PSU_5V) {  //&& getFullBridgeState() != FULL_BRIDGE_OFF
                                                  //if ((forwardRamp && getFullBridgeState() == FULL_BRIDGE_POSITIVE) || (!forwardRamp && getFullBridgeState() == FULL_BRIDGE_NEGATIVE)) {
    if (forwardRamp) {
      analogWrite(FB_A_H, rampValue);
    } else {
      analogWrite(FB_B_H, rampValue);
    }
    float current = readCurrentSenseCurrentFast();
    if (verboseLevel >= 2) {
      Serial.print(rampValue);
      Serial.print(" - ");
      Serial.println(current);
    }
    if (abs(current) > 0.2) {
      fullBridgeOff();
      rampEnabled = false;
      currentDetected = true;
      if (verboseLevel >= 2) Serial.println("Load detected");
      rampValue = 0;
    }
    if (rampValue == 2000) {
      rampValue = 0;
      fullBridgeOff();
      rampEnabled = false;
      currentDetected = false;
      if (verboseLevel >= 2) Serial.println("No load detected.");
    } else {
      rampValue++;
    }
    //}
  }
}

void sendPolarityStatusCommand(PolarityDetectType status) {
  if (verboseLevel >= 2) Serial.println("Sending PSU Status Message");
  tx_msg.id = CAN_IDENTIFIER;
  tx_msg.dlc = 8;
  tx_msg.data[0] = CAN_POLARITY_CHECK_DATA;
  tx_msg.data[1] = status;
  tx_msg.data[2] = CAN_STUFFING_FRAME;
  tx_msg.data[3] = CAN_STUFFING_FRAME;
  tx_msg.data[4] = CAN_STUFFING_FRAME;
  tx_msg.data[5] = CAN_STUFFING_FRAME;
  tx_msg.data[6] = CAN_STUFFING_FRAME;
  tx_msg.data[7] = CAN_STUFFING_FRAME;
  can2040.send_message(&tx_msg);
}