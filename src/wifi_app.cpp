#include "openchessboard.h"
#include <Logger.h>


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
  LOG_INFO << "Starting connection to server...";
 
  while (WiFi.status() == WL_CONNECTED){
    LOG_INFO << "Connected to Server...";
    LOG_INFO << "Find ongoing game";

    while(!is_game_running){
      getGameID(PostClient);
      
      //Start new game if no game is running and seek not already started
      if (board_gameMode != "None"  && !is_seeking &&  !is_game_running){
        LOG_INFO << "Wait for Starting Position";   
        while(!isStartingPosition()){
          delay(100);
        }
        
        LOG_INFO << "Start Game with preferred settings: " << board_gameMode;
        postNewGame(PostClient,  board_gameMode);
      } 
    }  

    getStream(StreamClient);   
    dimLEDs = false;
    timerFlag = true;
    String boardMove;

    while (is_game_running | is_seeking)
    {   
      moveStreamHandler();

      if (myturn & latestMove == oppLastMove){ 
        LOG_INFO << "Wait for accept move input...";
        while (boardMove != latestMove){
          displayMove(latestMove);

          boardMove = getMoveInput();
          String swapped_move = boardMove.substring(2, 4) + boardMove.substring(0, 2);
          if(boardMove.substring(0, 2) == boardMove.substring(2, 4) | swapped_move == latestMove){
            boardMove = latestMove;
            break;
          }
          LOG_INFO << "Move played on board: " << boardMove;

          if (boardMove != latestMove){
            displayMoveRecect(boardMove);
            break;
          }
        }

      } 

      if (myturn){
        LOG_INFO << "Wait for board move input...";
        bool moveSuccess = false; 
        while(is_game_running){ // wait for sucessful move transmission to get to opponents turn
          boardMove = getMoveInput();
          LOG_INFO << "Move played on board: " << boardMove;
          LOG_INFO << "Try to send move...";
          moveSuccess = postMove(PostClient, boardMove);
          bool once = true;
          String swapped_move;
          if (moveSuccess){
            myLastMove = boardMove;
            myturn = false;
            break;
          }
          else{
            if (once){
              swapped_move = boardMove.substring(2, 4) + boardMove.substring(0, 2);
              moveSuccess = postMove(PostClient,swapped_move);
              once = false;
            }
            if(moveSuccess){
              boardMove = swapped_move;
              myLastMove = boardMove;
              myturn = false;
              break;
            }
            LOG_INFO << "Invalid move. Wait for move take back...";
            displayMoveRecect(boardMove);
            boardMove = getMoveInput();
          }
        }
      }
    }
    
    byte frame[8];
    flickeringAnimation(frame);
    clearDisplay();
    LOG_INFO << "Game ended...";
    disableClient(StreamClient);
    disableClient(PostClient);
    WiFi.disconnect(true,true);
    ESP.restart();
  }  
}

