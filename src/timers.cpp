
#include "openchessboard.h"

hw_timer_t *timer = NULL;
volatile bool timerFlag = false;

// Interrupt Service Routine (ISR)
void IRAM_ATTR onTimer() {
  timerFlag = true;  // Set flag to indicate interrupt
}
              
void gameTimerHandler() {
  DEBUG_SERIAL.println(".");

  if (WiFi.status() != WL_CONNECTED){
    DEBUG_SERIAL.println("lost connection...restarting...");
    ESP.restart();
  }

  if (!StreamClient.available()){
    return;
  }

    char* char_response = catchResponseFromClient(StreamClient);
    //DEBUG_SERIAL.println(char_response);

    JsonDocument doc;
    String moves_temp;
    String latestMove_temp = "";
    String game_status = "";

    if (parseJsonResponse(char_response, doc)) {
        
        moves_temp = doc["state"]["moves"].as<String>();
        game_status = doc["state"]["status"].as<String>();

        if (moves_temp == "null" | moves_temp == "" ){
          moves_temp = doc["moves"].as<String>();
          game_status = doc["status"].as<String>();
        }

      	if (moves_temp == "null" | moves_temp == "" ){
          return;
        }

        moves = moves_temp;
        DEBUG_SERIAL.println(moves);

        latestMove = moves.substring(moves.length() - 4);

        if (latestMove == myLastMove){ 
          myturn = false;
        }
        else{
          myturn = true;
          oppLastMove = latestMove;
        }

        if (game_status == "started")
        {
          is_game_running = true;
        }
        
        if (moves.length() > 3 & game_status != "started"){
          is_game_running = false;
          is_seeking = false;
          myturn = false;
        }
          
    }

    
  
}


/* ---------------------------------------
    Function to set up interupt service routine.
    Sets blinking frequency by isr interval.
    @params[in] void
    @return void
*/
void isr_setup(void) {
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 100000, true);
  timerAlarmEnable(timer);
}

void disableGameTimer() {
  timerAlarmDisable(timer);
}

void enableGameTimer() {
  timerAlarmEnable(timer);
}
void moveStreamHandler() {
  if (timerFlag) {
    gameTimerHandler();      
    timerFlag = false;
  }
}