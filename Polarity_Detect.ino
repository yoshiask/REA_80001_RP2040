bool currentDetected = false;
bool rampEnabled = false;
bool forwardRamp = false;
uint16_t rampValue = 0;
bool forwardShort = false;
bool reverseShort = false;
bool runPolarityDetection = false;
uint16_t rampDelay = 0;
PolarityDetectType polarityDetectStatus = POLARITY_DETECT_NOT_RUN;
PolarityDetectType polarityDetectRequest = POLARITY_DETECT_NOT_RUN;
PSUState polarityDetectVoltageRequest = PSU_POWER_OFF;
PolarityDetectState polarityCurrentState = POL_IDLE;
PolarityDetectState polarityPreviousState = POL_IDLE;


void startRamp() {
  Serial.println("Starting Ramp");
  analogWriteFreq(10000);
  analogWriteRange(2000);
  rampValue = 0;
  rampEnabled = true;
}

void startPolarityDetect(PSUState voltageRequested, PolarityDetectType polarityRequested) {
  if (polarityRequested == POLARITY_FORWARD || polarityRequested == POLARITY_REVERSE) {
    polarityDetectRequest = polarityRequested;
    polarityDetectVoltageRequest = voltageRequested;
    runPolarityDetection = true;
    polarityCurrentState = POL_IDLE;
    setDesiredFullBridgeState(FULL_BRIDGE_OFF);
    setPSUState(PSU_POWER_OFF);
    analogWriteFreq(10000);
    analogWriteRange(10000);
  } else {
    Serial.println("Incorrect Polarity Detect Parameters Received");
  }
}

PolarityDetectType getPolarityDetectStatus() {
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
        if (polarityDetectRequest == POLARITY_FORWARD) {
          polarityCurrentState = POL_CHECK_FORWARD;
        }
        if (polarityDetectRequest == POLARITY_REVERSE) {
          polarityCurrentState = POL_CHECK_REVERSE;
        }
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
        setPSUState(polarityDetectVoltageRequest);
      }

      if (rampEnabled == false) {
        if (currentDetected) {
          forwardShort = true;
        } else {
          forwardShort = false;
        }
        polarityCurrentState = POL_IDLE;
        setPSUState(PSU_POWER_OFF);
        setDesiredFullBridgeState(FULL_BRIDGE_OFF);
        fullBridgeOff();
        if (!forwardShort) {
          Serial.println("Forward Polarity: No current detected");
          polarityDetectStatus = POLARITY_NO_DETECT;
        }
        if (forwardShort) {
          Serial.println("Reverse Polarity: Current detected");
          polarityDetectStatus = POLARITY_SHORTED;
        }
        sendPolarityStatusCommand(polarityDetectStatus);
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
        setPSUState(polarityDetectVoltageRequest);
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
        if (!reverseShort) {
          Serial.println("Reverse Polarity: No current detected");
          polarityDetectStatus = POLARITY_NO_DETECT;
        }
        if (reverseShort) {
          Serial.println("Reverse Polarity: Current Detected");
          polarityDetectStatus = POLARITY_SHORTED;
        }
        sendPolarityStatusCommand(polarityDetectStatus);
      }
      break;
  }
}

void rampHandler() {
  if (rampEnabled && getPSUStatus() != PSU_POWER_OFF) {  //&& getFullBridgeState() != FULL_BRIDGE_OFF
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