#include "OpenChessBoard.h"

/*
------------------------------------------------------------
User Settings START
Only change this part of the code for standard use
------------------------------------------------------------
*/
//Secret data, change to your credentials!
char ssid[] = "my_network";     // your network SSID (name), must be 2.4 GHz WiFi!!!
char pass[] = "my_password";    // your network password
char token[] = "my_lichess_API_token"; // your lichess API token
/*Note: When generating your token, make sure to provide all rights (all sliders set to green) */

/*
 * The standard orientation of the OPENCHESSBOARD is when the USB-Plug is on the right,
 * You can change the orientation by using the following define
*/
//#define PLUG_AT_TOP

/*
------------------------------------------------------------
User Settings END
------------------------------------------------------------
*/

// WiFi variables
int status = WL_IDLE_STATUS;
char server[] = "lichess.org";  // name address for lichess (using DNS)
WiFiSSLClient StreamClient; // WIFISSLClient for move stream, always connects via SSL (port 443 for https)
WiFiSSLClient PostClient; // WIFISSLClient for post moves, always connects via SSL (port 443 for https)

//lichess variables
const char* username;
const char* currentGameID;
bool myturn = true;
String lastMove;
String myMove;
bool is_castling_allowed = true;

// LED and state variables
bool boot_flipstate = true;
bool is_booting = true;
bool isr_first_run = false;
bool connect_flipstate = false;
bool is_connecting = false;
bool is_game_running = false;

void setup() {
  //Initialize HW
  initHW();
  isr_setup();


#if DEBUG == true
  //Initialize DEBUG_SERIAL and wait for port to open:
  DEBUG_SERIAL.begin(9600);
  delay(1000);
  while (!Serial);
#endif

  wifi_setup();

  DEBUG_SERIAL.println("\nStarting connection to server...");
}


void loop() {

  is_booting = false;
  is_connecting = true;
  isr_first_run = false;
  lastMove = "xx";
  myMove = "ff";

  PostClient.connect(server, 443);
  DEBUG_SERIAL.println("\nConnected to Server...");

  if (StreamClient.connect(server, 443))
  {
    DEBUG_SERIAL.println("Find ongoing game");

    getGameID(StreamClient); // checks whos turn it is

    if (currentGameID != NULL)
    {
      DEBUG_SERIAL.println("Start move stream from game");
      getStream(StreamClient);

      delay(500);// make sure first move is catched by isr

      is_game_running = true;
      is_connecting = false;

      while (is_game_running)
      {
        while(!myturn && is_game_running){

          //isr parses move stream once a second, exits when game ends or myturn is set to true
          delay(300);
        }

        if (myturn && is_game_running && isr_first_run)
        {
          String accept_move = "none";

          //print last move if move was detected
          if (lastMove.length() > 3){
            DEBUG_SERIAL.print("opponents move: ");
            DEBUG_SERIAL.println(lastMove);

            // wait for oppents move to be played
            DEBUG_SERIAL.println("wait for move accept...");

            while(accept_move != lastMove && is_game_running){
              displayMove(lastMove);
              accept_move = getMoveInput();

              // if king move is a castling move, wait for rook move
              checkCastling(accept_move);

              }



            DEBUG_SERIAL.println("move accepted!");
          }

          // run isr at least once to catch first move of the game
          if(isr_first_run){
            //wait for my move and send it to API
            DEBUG_SERIAL.print("wait for move input...");
            postMove(PostClient);
          }
        }
      }
    }

    StreamClient.stop();
  }
}
