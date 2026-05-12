#include "openchessboard.h"
#include <ArduinoBleOTA.h>
#include <ArduinoBleChess.h>
#include <ArduinoBleChessMultiservice.h>
#include <Logger.h>
#define DEVICE_NAME "OCB" // max name size with 128 bit uuid is 11

bool skip_next_send = false;
bool my_castling_rights = true;
bool opp_castling_rights = true;
bool first_run = true;
bool forceSync = false;

bool isCastling(BleChessString move_input) {
  // check if last move was king move from castling
  if(((move_input == "e1g1") || (move_input == "e1c1") ||  (move_input == "e8g8") || (move_input == "e8c8")) ){
    
    if (my_castling_rights)
    {
      my_castling_rights = false;
      LOG_INFO << "My castle move...";
      return true;
    }
    if (opp_castling_rights)
    {
      opp_castling_rights = false;
      LOG_INFO << "Opponent castle move...";
      return true;
    }
  }
  return false;
}

class Peripheral : public BleChessPeripheral
{
public:
  void handleCentralFeature(BleChessStringView feature) override {
    const bool isSuppported =
      feature == BleChessFeature::Msg ||
      feature == BleChessFeature::LastMove ||
      feature == BleChessFeature::Check ||
      feature == BleChessFeature::Side ||
      feature == BleChessFeature::GetState ||
      feature == BleChessFeature::VariantReason;
    sendPeripheralAck(isSuppported);
    LOG_INFO << "Feature: " << feature << (isSuppported ? " supported" : " unsupported");
  }

  void handleCentralVariant(BleChessStringView variant) override {
    const bool isSuppported =
      variant == BleChessVariant::Standard ||
      variant == BleChessVariant::ThreeCheck ||
      variant == BleChessVariant::Atomic ||
      variant == BleChessVariant::KingOfTheHill ||
      variant == BleChessVariant::RacingKings;
    sendPeripheralAck(isSuppported);
    LOG_INFO << "Variant: " << variant << (isSuppported ? " supported" : " unsupported");
  }

  void handleCentralGetState() override {
    LOG_INFO << "Get state";
    sendPeripheralState(createFen().c_str());
  }

  void handleCentralSetVariant(BleChessStringView variant) override {
    LOG_INFO << "Set variant: " << variant;
  }

  void handleCentralSide(BleChessStringView side) override {
    LOG_INFO << "Side: " << side;

    if (side == BleChessSide::White) {
      LOG_INFO << "White side";
    } else if (side == BleChessSide::Black) {
      LOG_INFO << "Black side";
    } else if (side == BleChessSide::Both) {
      LOG_INFO << "Both sides";
    }
  }

  void handleCentralBegin(BleChessStringView fen) override {
    clearDisplay();
    displayNewGame();
    is_game_running = true;
    LOG_INFO << "Begin: " << fen;
    
    String peripheralFen = createFen();
    centralFen = fen;
    if (forceSync){
      peripheralFen = centralFen.c_str(); 
    }
    isSynchronized = areFensSame(peripheralFen, centralFen.c_str());
    LOG_INFO << "Peripheral fen: " << peripheralFen;
    isSynchronized ?
      sendPeripheralSync(peripheralFen.c_str()) :
      sendPeripheralUnsync(peripheralFen.c_str());
    LOG_INFO << (isSynchronized ? "Synchronized" : "Unsynchronized");
  }

  void handleCentralMove(BleChessStringView mv) override {
    clearDisplay();
    if (is_game_running){
      LOG_INFO << "Central move: " << mv;
      //synchronize();
      const auto str = String(mv.data(), mv.length());
      displayMove(str);
      oppLastMove = str;
      skip_next_send = true;
    }
  }

  void handlePeripheralMoveAck(bool ack) override {
    LOG_DEBUG << "Move ack: " << ack;
    clearDisplay();
    ack ?
      handleMoveAccepted() :
      handleMoveRejected();
  }

  void handlePeripheralMovePromoted(BleChessStringView mv) override {
    LOG_INFO << "Promoted: " << mv;
  }

  void handleCentralEnd(BleChessStringView reason) override {
    clearDisplay();
    is_game_running = false;
    LOG_INFO << "End: " << reason;

    if (reason == BleChessEndReason::Checkmate) {
      LOG_INFO << "Checkmate";
    } else if (reason == BleChessEndReason::Draw) {
      LOG_INFO << "Draw";
    } else if (reason == BleChessEndReason::Timeout) {
      LOG_INFO << "Timeout";
    } else if (reason == BleChessEndReason::Resign) {
      LOG_INFO << "Resign";
    } else if (reason == BleChessEndReason::Abort) {
      LOG_INFO << "Abort";
    } else if (reason == BleChessEndReason::Undefined) {
      LOG_INFO << "Variant end";
    } else if (reason == BleChessVariantReason::ThreeCheck) {
      LOG_INFO << "Three check";
    } else if (reason == BleChessVariantReason::KingOfTheHill) {
      LOG_INFO << "King of the hill";
    }
  }

  void handleCentralLastMove(BleChessStringView mv) override {
    clearDisplay();
    LOG_INFO << "Last move: " << mv;
    const auto str = String(mv.data(), mv.length());
    displayMove(str);
    oppLastMove = str;
  }

  void handleCentralCheck(BleChessStringView kingPos) override {
    LOG_INFO << "Check: " << kingPos;
  }

  void handleMoveAccepted() {
    if (is_game_running) {
      clearDisplay();
      LOG_INFO << "Move accepted: " << lastPeripheralMove;
      //displayMove(lastPeripheralMove.c_str());
      skip_next_send = false;
      // my_castling_rights = true;
      // opp_castling_rights = true;
    }
  }

  void handleMoveRejected() {
    if (is_game_running) {
      LOG_INFO << "Move rejected: " << lastPeripheralMove;
      for (int k = 0; k < 3; k++){
        clearDisplay();
        delay(200);
        displayMove(lastPeripheralMove.c_str());
        delay(200); 
      }
      skip_next_send = true; /* give one try to reverse move, before sending next move to central */
    }
  }
  
  void synchronize() {
    LOG_DEBUG << "Synchronize";
    if (!isSynchronized) {
      LOG_INFO << "Synchronized";
      sendPeripheralSync(createFen().c_str());
    }
    isSynchronized = true;
  }

  void checkPeripheralMove() {
    LOG_DEBUG << "Check peripheral move";
    if (!isSynchronized) {
      String peripheralFen = createFen();
      if (forceSync){
        peripheralFen = centralFen.c_str(); 
      }
      isSynchronized = areFensSame(peripheralFen, centralFen.c_str());
      if (!isSynchronized) {
        sendPeripheralState(peripheralFen.c_str());
        delay(300);
        return;
      }
      LOG_INFO << "Synchronized";
      sendPeripheralSync(peripheralFen.c_str());
    }

    BleChessString move = getMoveInput().c_str();

    if(isCastling(move)){
      getMoveInput(); /* get second move from castling but do not send it: send king move only after second input */
    }

    LOG_INFO << "Peripheral move: " << move;
    
    clearDisplay();
    if (!skip_next_send & move != "") {
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
    LOG_INFO << "BLE Init: OPENCHESSBOARD";
    const auto server = initBle(DEVICE_NAME);
    if (!ArduinoBleChess.begin(server, peripheral)){
      LOG_ERROR << "Ble Chess initialization error";
    }
    if (!ArduinoBleOTA.begin(server, InternalStorage)) {
      LOG_ERROR << "Ble OTA initialization error";
    }
    advertiseBle(server, DEVICE_NAME, BLE_CHESS_SERVICE_UUID, BLE_OTA_SERVICE_UUID);
    LOG_INFO << "Start BLE polling...";
    first_run = false;
  }

  while(!is_game_running){
    ArduinoBleOTA.pull();
    displayWaitForGame();
  }
  while(true){ // while BLE connected
    peripheral.checkPeripheralMove();
  }
}