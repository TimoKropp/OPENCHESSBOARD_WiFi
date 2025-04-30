#include "openchessboard.h"
#include <ArduinoBleOTA.h>
#include <ArduinoBleChess.h>
#include <BleChessUuids.h>
#include <BleOtaUuids.h>
#include <BleChessMultiservice.h>
#include <BleChessData.h>
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
    const bool isSuppported =
      feature == BleChessFeature::Msg ||
      feature == BleChessFeature::LastMove ||
      feature == BleChessFeature::Check ||
      feature == BleChessFeature::Side ||
      feature == BleChessFeature::GetState ||
      feature == BleChessFeature::VariantReason;
    sendPeripheralAck(isSuppported);
    DEBUG_SERIAL.print("feature: ");
    DEBUG_SERIAL.print(feature.c_str());
    DEBUG_SERIAL.println(isSuppported ? " supported" : " unsupported");
  }

  void onCentralVariant(const BleChessString& variant) override {
    const bool isSuppported =
      variant == BleChessVariant::Standard ||
      variant == BleChessVariant::ThreeCheck ||
      variant == BleChessVariant::Atomic ||
      variant == BleChessVariant::KingOfTheHill ||
      variant == BleChessVariant::RacingKings;
    sendPeripheralAck(isSuppported);
    DEBUG_SERIAL.print("variant: ");
    DEBUG_SERIAL.print(variant.c_str());
    DEBUG_SERIAL.println(isSuppported ? " supported" : " unsupported");
  }

  void onCentralGetState() override {
    DEBUG_SERIAL.print("get state");
    sendPeripheralState(createFen().c_str());
  }

  void onCentralSetVariant(const BleChessString& variant) override {
    DEBUG_SERIAL.print("set variant: ");
    DEBUG_SERIAL.println(variant.c_str());
  }

  void onCentralSide(const BleChessString& side) override {
    DEBUG_SERIAL.print("side: ");
    DEBUG_SERIAL.println(side.c_str());

    if (side == BleChessSide::White) {
      DEBUG_SERIAL.print("white side");
    } else if (side == BleChessSide::Black) {
      DEBUG_SERIAL.print("black side");
    } else if (side == BleChessSide::Both) {
      DEBUG_SERIAL.print("both sides");
    }
  }

  void onCentralBegin(const BleChessString& fen) override {
    clearDisplay();
    displayNewGame();
    is_game_running = true;
    DEBUG_SERIAL.print("begin: ");
    DEBUG_SERIAL.println(fen.c_str());
    
    String peripheralFen = createFen();
    centralFen = fen;
    if (forceSync){
      peripheralFen = centralFen.c_str(); 
    }
    isSynchronized = areFensSame(peripheralFen, centralFen.c_str());
    DEBUG_SERIAL.print("peripheralFen: ");
    DEBUG_SERIAL.println(peripheralFen);
    isSynchronized ?
      sendPeripheralSync(peripheralFen.c_str()) :
      sendPeripheralUnsync(peripheralFen.c_str());
    DEBUG_SERIAL.print(isSynchronized ? "synchronized" : "unsynchronized");
  }

  void onCentralMove(const BleChessString& mv) override {
    clearDisplay();
    if (is_game_running){
      DEBUG_SERIAL.print("central move: ");
      DEBUG_SERIAL.println(mv.c_str());
      //synchronize();
      displayMove(mv.c_str());
      oppLastMove = mv.c_str();
      skip_next_send = true;
    }
  }

  void onPeripheralMoveAck(bool ack) override {
    //DEBUG_SERIAL.print("trace: onPeripheralMoveAck");
    clearDisplay();
    ack ?
      onMoveAccepted() :
      onMoveRejected();
  }

  void onPeripheralMovePromoted(const BleChessString& mv) override {
    DEBUG_SERIAL.print("promoted: ");
    DEBUG_SERIAL.println(mv.c_str());
  }

  void onCentralEnd(const BleChessString& reason) override {
    clearDisplay();
    is_game_running = false;
    DEBUG_SERIAL.print("end: ");
    DEBUG_SERIAL.println(reason.c_str());

    if (reason == BleChessEndReason::Checkmate) {
      DEBUG_SERIAL.print("checkmate");
    } else if (reason == BleChessEndReason::Draw) {
      DEBUG_SERIAL.print("draw");
    } else if (reason == BleChessEndReason::Timeout) {
      DEBUG_SERIAL.print("timeout");
    } else if (reason == BleChessEndReason::Resign) {
      DEBUG_SERIAL.print("resign");
    } else if (reason == BleChessEndReason::Abort) {
      DEBUG_SERIAL.print("abort");
    } else if (reason == BleChessEndReason::Undefined) {
      DEBUG_SERIAL.print("variant end");
    } else if (reason == BleChessVariantReason::ThreeCheck) {
      DEBUG_SERIAL.print("tree check");
    } else if (reason == BleChessVariantReason::KingOfTheHill) {
      DEBUG_SERIAL.print("king of the hill");
    }
  }

  void onCentralLastMove(const BleChessString& mv) override {
    clearDisplay();
    DEBUG_SERIAL.print("last move: ");
    DEBUG_SERIAL.println(mv.c_str());
    displayMove(mv.c_str());
    oppLastMove = mv.c_str();
  }

  void onCentralCheck(const BleChessString& kingPos) override {
    DEBUG_SERIAL.print("check: ");
    DEBUG_SERIAL.println(kingPos.c_str());
  }

  void onMoveAccepted() {
    if (is_game_running) {
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
    if (is_game_running) {
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
  
  void synchronize() {
    //DEBUG_SERIAL.print("trace: synchronize");
    if (!isSynchronized) {
      sendPeripheralSync(createFen().c_str());
    }
    isSynchronized = true;
  }

  void checkPeripheralMove() {
    //DEBUG_SERIAL.print("trace: checkPeripheralMove");
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
      DEBUG_SERIAL.print("synchronized");
      sendPeripheralSync(peripheralFen.c_str());
    }

    BleChessString move = getMoveInput().c_str();

    if(isCastling(move)){
      getMoveInput(); /* get second move from castling but do not send it: send king move only after second input */
    }

    DEBUG_SERIAL.print("peripheral move: ");
    DEBUG_SERIAL.println(move.c_str());
    
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

  while(!is_game_running){
    ArduinoBleOTA.pull();
    displayWaitForGame();
  }
  while(true){ // while BLE connected
    peripheral.checkPeripheralMove();
  }
}