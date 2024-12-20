#include <Adafruit_NeoPixel.h>
#include <ACAN2040.h>
#include <Wire.h>
#include "Adafruit_HUSB238.h"

#define LED_STATUS_PIN 9
#define LED_STATUS_ADDRESS 0
#define LED_PD_STATUS_ADDRESS 1
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1
#define PSU_CONNECT_OUTPUT_PIN 2  // Enable switch on the output of the power supply, active high
#define PSU_STANDBY_PIN 3         // Turn off the power supply output, active high
#define PSU_EN_12V_PIN 4          // High - 12V power supply output, Low - 5V power supply output
#define CONNECT_INPUT_PIN 5       // Connects the input supply directly to the output - active high
#define FB_B_H 10                 // Enable for negative connection
#define FB_B_L 11                 // Enable for negative connection
#define FB_A_H 12                 // Enable for positive connection
#define FB_A_L 13                 // Enable for positive connection
#define DEVICE_TYPE_PIN_0 24      // Device ID Pin 0
#define DEVICE_TYPE_PIN_1 25      // Device ID Pin 1
#define CURRENT_SENSE A0
#define CC1_UFP A1
#define CC1_DFP A2
#define CC2_DFP A3
#define CAN_NTERM 8


#define CAN_STUFFING_FRAME 0xAA
#define CAN_IDENTIFIER 0x0A

#define WIRE Wire

const uint8_t PIONUM0 = 0;
const uint8_t TXPIN0 = 7;
const uint8_t RXPIN0 = 6;
const uint32_t BITRATE0 = 125000UL;
const uint32_t SYSCLK = F_CPU;

void my_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg);
char *msg_to_str(struct can2040_msg *msg);

ACAN2040 can2040(PIONUM0, TXPIN0, RXPIN0, BITRATE0, SYSCLK, my_cb);
bool got_msg = false;
struct can2040_msg tx_msg, rx_msg;
struct can2040_stats can_stats;

enum PSUState {
  PSU_POWER_OFF = 1,
  PSU_20V = 2,
  PSU_12V = 3,
  PSU_5V = 4
};

enum PSUStatus {
  PSU_OK = 0,
  PSU_OVER_CURRENT = 1
};

enum CANDataType {
  CAN_PSU_VOLTAGE = 1,
  CAN_TEST_PATTERN = 2,
  CAN_CURRENT_REQUEST = 3,
  CAN_CURRENT_ZERO_REQUEST = 4,
  CAN_CURRENT_DATA = 5,
  CAN_PSU_STATUS = 6,
  CAN_OUTPUT_POLARITY = 7
};

enum DeviceType {
  DEVICE_UNKNOWN = 0,
  DEVICE_ATTACHED_PSU = 1,
  DEVICE_STANDALONE_PSU = 2
};

// Enum for full bridge states
enum FullBridgeType {
  FULL_BRIDGE_OFF,
  FULL_BRIDGE_POSITIVE,
  FULL_BRIDGE_NEGATIVE
};

enum FullBridgePolarity {
  FULL_BRIDGE_POLARITY_POSITIVE,
  FULL_BRIDGE_POLARITY_NEGATIVE
};

PSUState psuState = PSU_POWER_OFF;

Adafruit_NeoPixel status_led = Adafruit_NeoPixel(2, LED_STATUS_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  initializeSerial();
  delay(100);
  initializeStatusLED();
  checkDeviceType();
  can2040.begin();
  initI2C();
  initalize_USB_PD();
  initCurrentSense();
  initializePSUPins();
  powerStateMachineCommand(PSU_POWER_OFF);
  initializeFullBridge();
  terminateCAN();

  //To be reconsidered
  requestPDProfile();
}


void loop() {
  slottedLoop();
}


//Functions that run once every 100ms
void Slot_100ms() {
  refreshStatusLED();
  fullBridgeStateMachine();
  ledHandler();
  checkInputVoltage();
}

//Functions that run once every 10ms
void Slot_10ms() {
  checkCANMessages();
  serialParser();
}

//Functions that run once every loop (the fastest possible)
void Slot_EveryLoop() {
}
