/* ---------------------------------------
 * This is the main file of the OpenChessBoard firmware v1.0.0
*/   

//Includes
#include <WiFiNINA.h>
#include <ArduinoJson.h>


// WiFi variables
int status = WL_IDLE_STATUS;
char server[] = "lichess.org";  // name address for lichess (using DNS)
WiFiSSLClient StreamClient; // WIFISSLClient for move stream, always connects via SSL (port 443 for https)
WiFiSSLClient PostClient; // WIFISSLClient for post moves, always connects via SSL (port 443 for https)


//Secret data, change to your credentials!
char ssid[] = "Board";     // your network SSID (name)
char pass[] = "board123";    // your network password (use for WPA, or use as key for WEP)
char token[] = "my_lichess_token"; // your lichess API token : Arduino_Lichess, botmokko2


//lichess variables
const char* username;
const char* currentGameID;
bool myturn = true;
String lastMove;
String myMove;


// LED and state variables
bool boot_flipstate = true;
bool is_booting = true;
bool isr_first_run = false;;
bool connect_flipstate = false;
bool is_connecting = false;
bool is_game_running = false;


// Debug Settings
#define DEBUG true  //set to true for debug output, false for no debug output
#define DEBUG_SERIAL if(DEBUG)Serial

void setup() {
  //Initialize HW
  initHW();
  isr_retup();

#if DEBUG == true
  //Initialize DEBUG_SERIAL and wait for port to open:
  DEBUG_SERIAL.begin(9600);
  delay(1000);
  while (!Serial);
#endif  

  wifi_setup();
  
  DEBUG_SERIAL.println("\nStarting connection to server...");
  
  if (StreamClient.connect(server, 443))
  {
    getUsername(StreamClient);
  }
  else {
    DEBUG_SERIAL.print("no connection to server");
  }
  
}


void loop() {

  is_booting = false;
  is_connecting = true;
  lastMove = "xx";
  myMove = "ff";
  
  if (StreamClient.connect(server, 443))
  {
    DEBUG_SERIAL.println("Find ongoing game");
    
    getGameID(StreamClient); // checks whos turn it is

    if (currentGameID != NULL)
    {
      DEBUG_SERIAL.println("Start move stream from game");
      getStream(StreamClient);
      
      //PostClient.connect(server, 443);
      
      is_game_running = true;
      is_connecting = false;
        
      while (is_game_running)
      {   
        while(!myturn && is_game_running){
        
          //isr parses move stream once a second, exits when game ends or myturn is set to true
          delay(100);
        }
        
        if (myturn && is_game_running)
        { 
          //print last move if move was detected
          if (lastMove.length() > 3 && isr_first_run){
            DEBUG_SERIAL.print("opponents move: ");
            DEBUG_SERIAL.println(lastMove);

            // wait for oppents move to be played
            DEBUG_SERIAL.print("wait for move accept...");
            String accept_move = "none";        
            while(accept_move != lastMove && is_game_running){
              displayMove(lastMove);
              accept_move = getMoveInput();
              }
            DEBUG_SERIAL.print("move accepted!");
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
  }
}
