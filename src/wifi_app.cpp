#include "openchessboard.h"



// LED and state variables
bool update_flipstate = true;
bool is_booting = true;
bool is_updating = false;
bool is_connecting = false;
bool is_game_running = false;
bool is_seeking = false;
bool dimLEDs  = false;

// WiFi, and timer variables
String wifi_ssid;
String wifi_password;
int status = WL_IDLE_STATUS;
WiFiClientSecure StreamClient;
WiFiClientSecure PostClient;

//lichess variables
String lichess_api_token;
char server[] = "lichess.org"; 
String username = "no";
String currentGameID = "noGame";
bool myturn = false;
String lastMove = "no";
String myMove = "no";
String moves = "no";
bool is_castling_allowed = true;


void run_WiFi_app(void){
  setStateConnecting();

  wifi_setup();
  wifi_firmwareUpdate();

  PostClient.setInsecure();
  StreamClient.setInsecure();
  DEBUG_SERIAL.println("\nStarting connection to server...");
  
  while (1){
    DEBUG_SERIAL.println("\nConnected to Server...");
    DEBUG_SERIAL.println("Find ongoing game");

    while(!is_game_running){
      getGameID(StreamClient);
      //Start new game if no game is running and seek not already started
      if (board_gameMode != "None"  && !is_seeking &&  !is_game_running){
        DEBUG_SERIAL.println("\nWait for Starting Position");   
        while(!isStartingPosition()){
          delay(100);
        }
        
        DEBUG_SERIAL.println("\nStart Game with prefered settings: "+ board_gameMode);   
        postNewGame(PostClient,  board_gameMode);
      } 
    }  
    dimLEDs = false;
    getStream(StreamClient);

    while (is_game_running)
    {   
        moveStreamHandler();

        if (myturn && is_game_running)
        {
        String accept_move = "no";  
        
        //print last move if move was detected
        if (lastMove.length() == 4){
            DEBUG_SERIAL.print("opponents move: ");
            DEBUG_SERIAL.println(lastMove);

            // wait for oppents move to be played
            DEBUG_SERIAL.println("wait for move accept...");
    
            while(accept_move != lastMove && is_game_running){
            displayMove(lastMove);
            accept_move = getMoveInput();
            // if king move is a castling move, wait for rook move
            checkCastling(accept_move);
            clearDisplay();
            }

            DEBUG_SERIAL.println("move accepted!");
        }

        // blocking, waits for move input
        postMove(PostClient);
        }
    }
  }
}
