
#include "openchessboard.h"

hw_timer_t *timer = NULL;
volatile bool timerFlag = false;
String game_status;

// Interrupt Service Routine (ISR)
void IRAM_ATTR onTimer() {
  timerFlag = true;  // Set flag to indicate interrupt
}
              
void timerHandler() {
  DEBUG_SERIAL.println(".");
  if (is_booting)
  {
    displayBootWait();
    boot_flipstate = !boot_flipstate;
  }

  if (is_connecting)
  { 
    displayConnectWait();
    connect_flipstate = !connect_flipstate;
  }
  
  if (is_game_running && !is_booting && !is_connecting)
  { 
    
    char* char_response = catchResponseFromClient(StreamClient);

    moves = parseValueFromResponse(char_response, "moves");
    game_status = parseValueFromResponse(char_response, "status");

    // Detect Game restart
    if (game_status != "started" && game_status != "no")
    {
      DEBUG_SERIAL.print("Game Status: ");
      DEBUG_SERIAL.println(game_status);
      setStateConnecting();
      return;
    }

    // Check Move
    if (moves.length() > 3)
    {
      DEBUG_SERIAL.print("move received: ");
      int startstr = moves.length() - 4; 
      lastMove = moves.substring(startstr);
      DEBUG_SERIAL.println(lastMove);
    }
    
    if (lastMove != myMove)
    {
      myturn = true;
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
  timerAlarmWrite(timer, 300000, true);
  timerAlarmEnable(timer);
}

void disableISR() {
  timerAlarmDisable(timer);
}

void enableISR() {
  timerAlarmEnable(timer);
}
