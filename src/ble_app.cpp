#include "OpenChessBoard.h"
#include <ArduinoBleOTA.h>
#include <ArduinoBleChess.h>
#include <BleChessUuids.h>
#include <BleOtaUuids.h>
#include <BleChessMultiservice.h>
#define DEVICE_NAME "OCB" // max name size with 128 bit uuid is 11

bool skip_next_send = false;
bool my_castling_rights = true;
bool opp_castling_rights = true;

bool game_running = false;
bool first_run = true;

bool isCastling(BleChessString move_input) {
  // check if last move was king move from castling
  if(((move_input == "e1g1") || (move_input == "e1c1") ||  (move_input == "e8g8") || (move_input == "e8c8")) ){
    
    if (my_castling_rights)
    {
      my_castling_rights = false;
      DEBUG_SERIAL.println("my castle move...");
      return true;
    }
    if (opp_castling_rights)
    {
      opp_castling_rights = false;
      DEBUG_SERIAL.print("opponent castle move...");
      return true;
    }
  }
  return false;
}

class Peripheral : public BleChessPeripheral
{
public:
  void onCentralFeature(const BleChessString& feature) override {
    clearDisplay();
    const bool isSuppported =
      feature == "msg" ||
      feature == "last_move";
    sendPeripheralAck(isSuppported);
  }

  void onCentralFen(const BleChessString& fen) override {
    clearDisplay();
    displayNewGame();
    game_running = true;
    DEBUG_SERIAL.print("new game: ");
    DEBUG_SERIAL.println(fen.c_str());
    
    String peripheralFen = getFen();
    centralFen = fen;
    isSynchronized = areFensSame(peripheralFen, centralFen.c_str());
    if (!isSynchronized) {
      sendPeripheralAck(false);
      sendPeripheralFen(peripheralFen.c_str());
    }
    sendPeripheralAck(true);
  }

  void onPeripheralFenAck(bool ack) override {
  }

  void onCentralMove(const BleChessString& mv) override {
    clearDisplay();
    if (game_running){
      DEBUG_SERIAL.print("moved from central: ");
      DEBUG_SERIAL.println(mv.c_str());
      displayMove(mv.c_str());
      skip_next_send = true;
    }
    sendPeripheralAck(true);
  }

  void onPeripheralMoveAck(bool ack) override {
    if (ack){
      clearDisplay();
      onMoveAccepted();
    }
    else{
      clearDisplay();
      onMoveRejected();
    }
  }

  void onPeripheralMovePromoted(const BleChessString& mv) override {
    DEBUG_SERIAL.print("promoted on central screen: ");
    DEBUG_SERIAL.println(mv.c_str());

    sendPeripheralAck(true);
  }

  void onCentralLastMove(const BleChessString& mv) override {
    clearDisplay();
    DEBUG_SERIAL.print("last move: ");
    DEBUG_SERIAL.println(mv.c_str());
    displayMove(mv.c_str());

    sendPeripheralAck(true);
  }

  void onMoveAccepted() {
    if (game_running) {
      clearDisplay();
      DEBUG_SERIAL.print("move accepted: ");
      DEBUG_SERIAL.println(lastPeripheralMove.c_str());
      //displayMove(lastPeripheralMove.c_str());
      skip_next_send = false;
      // my_castling_rights = true;
      // opp_castling_rights = true;
    }
  }

  void onMoveRejected() {
    if (game_running) {
      DEBUG_SERIAL.print("move rejected: ");
      DEBUG_SERIAL.println(lastPeripheralMove.c_str());
      for (int k = 0; k < 3; k++){
        clearDisplay();
        delay(200);
        displayMove(lastPeripheralMove.c_str());
        delay(200); 
      }
      skip_next_send = true; /* give one try to reverse move, before sending next move to central */
    }
  }
  
  void checkPeripheralMove() {

    BleChessString move = getMoveInput().c_str();

    if(isCastling(move)){
      getMoveInput(); /* get second move from castling but do not send it: send king move only after second input */
    }

    DEBUG_SERIAL.print("moved from peripheral: ");
    DEBUG_SERIAL.println(move.c_str());
    
    clearDisplay();
    if (!skip_next_send){
      sendPeripheralMove(move);
      lastPeripheralMove = move;
    }
    skip_next_send = false; /* skip only once, then allow new move send */
  }

private:
  BleChessString lastPeripheralMove;
  BleChessString centralFen;
  bool isSynchronized = true;
};
Peripheral peripheral{};

void run_BLE_app(){
  if (first_run){
    DEBUG_SERIAL.println("BLE init: OPENCHESSBOARD");
    initBle(DEVICE_NAME);
    if (!ArduinoBleChess.begin(peripheral)){
      DEBUG_SERIAL.println("Ble chess initialization error");
    }
    if (!ArduinoBleOTA.begin(InternalStorage)) {
      DEBUG_SERIAL.println("Ble ota initialization error");
    }
    advertiseBle(DEVICE_NAME, BLE_CHESS_SERVICE_UUID, BLE_OTA_SERVICE_UUID);
    DEBUG_SERIAL.println("start BLE polling...");
    first_run = false;
  }
  #ifndef USE_NIM_BLE_ARDUINO_LIB
    BLE.poll();
  #endif

  while(!game_running){
    ArduinoBleOTA.pull();
    displayWaitForGame();
  }
  peripheral.checkPeripheralMove();
}