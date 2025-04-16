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
String myLastMove = "xx";
String oppLastMove = "xy";
String latestMove = "zz";
String myMove = "yy";
String moves = "";
bool is_castling_allowed = true;


void run_WiFi_app(void){

  setStateConnecting();
  wifi_setup();
  wifi_firmwareUpdate();

  PostClient.setInsecure();
  StreamClient.setInsecure();
  DEBUG_SERIAL.println("\nStarting connection to server...");
 
  while (WiFi.status() == WL_CONNECTED){
    DEBUG_SERIAL.println("\nConnected to Server...");
    DEBUG_SERIAL.println("Find ongoing game");

    while(!is_game_running){
      getGameID(PostClient);
      
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

    getStream(StreamClient);   
    dimLEDs = false;
    timerFlag = true;
    String boardMove = oppLastMove;

    while (is_game_running | is_seeking)
    {   
      moveStreamHandler();

      if (myturn & latestMove == oppLastMove){ 
        DEBUG_SERIAL.println("Wait for accept move input...");
        while (boardMove != latestMove){
          displayMove(latestMove);

          boardMove = getMoveInput();
          DEBUG_SERIAL.print("move played on board: ");
          DEBUG_SERIAL.println(boardMove);
          if (boardMove != latestMove){
            displayMoveRecect(boardMove);
            break;
          }
        }

      } 

      if (myturn & boardMove == oppLastMove){
        DEBUG_SERIAL.println("Wait for board move input...");
        bool moveSuccess = false; 
        while(true){ // wait for sucessful move transmission to get to opponents turn
          boardMove = getMoveInput();
          DEBUG_SERIAL.print("move played on board: ");
          DEBUG_SERIAL.println(boardMove);
          DEBUG_SERIAL.println("try to send move...");
          moveSuccess = postMove(PostClient, boardMove);

          if (moveSuccess){
            myLastMove = boardMove;
            myturn = false;
            break;
          }
          else{
            DEBUG_SERIAL.println("invalid move. wait for move take back...");
              displayMoveRecect(boardMove);
              boardMove = getMoveInput();
          }
        }
      }
    }
    
    byte frame[8];
    flickeringAnimation(frame);
    clearDisplay();
    DEBUG_SERIAL.println("game ended...");
    ESP.restart();
  }  
}

