#include "openchessboard.h"
#include <Logger.h>

//board configuration
String board_gameMode;
String board_startupType;


void setup() {
  initHW();
  isr_setup();
  
#ifndef LOG_LVL_DISABLED
  //Initialize Serial and wait for port to open:
  Serial.begin(115200);
  while (not Serial);
#endif

validateFirmware(); // runs wifi as check

readSettings();
readBoardSelection();

if (board_startupType == "WiFi") {
   LOG_INFO << "Run WiFi App...";
   run_WiFi_app();
}
else if (board_startupType == "BLE") {
  LOG_INFO << "Run BLE App...";
  run_BLE_app();
}
else if (board_startupType == "PUZZLE") {
  disableGameTimer();
  LOG_INFO << "Run Queen Puzzle App...";
  run_queen_puzzle();
}
else{
    LOG_INFO << "Run Access Point to Fetch Settings...";
    run_APsettings(); // runs until settings are submitted
}

}


void loop() {

}