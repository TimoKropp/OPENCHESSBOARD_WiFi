//Includes
#include <WiFiNINA.h>
#include <ArduinoJson.h>


// WiFi variables
int status = WL_IDLE_STATUS;
char server[] = "lichess.org";  // name address for lichess (using DNS)
WiFiSSLClient StreamClient; // WIFISSLClient for move stream, always connects via SSL (port 443 for https)
WiFiSSLClient PostClient; // WIFISSLClient for post moves, always connects via SSL (port 443 for https)


//Secret data, change to your credentials!
char ssid[] = "my_network_name";     
char pass[] = "my_network_password";  
char token[] = "my_lichess_api_token"; 


//lichess variables
const char* username;
const char* currentGameID;
bool myturn = true;
String lastMove;
String myMove;


// LED and state variables
bool boot_flipstate = true;
bool is_booting = true;
bool connect_flipstate = false;
bool is_connecting = false;
bool is_game_running = false;


// Debug Settings
#define DEBUG false  //set to true for debug output, false for no debug output
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
  
  if (StreamClient.connect(server, 443))
  {
    DEBUG_SERIAL.println("Find ongoing game");
    getGameID(StreamClient);
    
    if (currentGameID != NULL)
    {
      is_game_running = true;
      is_connecting = false;
        
      DEBUG_SERIAL.println("Start stream and wait for next move");
      getStream(StreamClient);
      DEBUG_SERIAL.println("wait for incoming moves");
      
      while (is_game_running)
      {
       while (!myturn && is_game_running){
         
         char char_response[800] = {0};
         
         while(StreamClient.available()) {
          char_response[800] = {0};
          StreamClient.readBytesUntil('\n', char_response, sizeof(char_response));
         }
          
          String moves  = GetStringBetweenStrings((String)char_response, "moves", "wtime");

          if (moves.length() > 1)
          {
            int startstr = moves.length() - 7;
            int endstr =  moves.length() - 3;
            lastMove = moves.substring(startstr, endstr);
          }
          if (lastMove != myMove)
          {
            myturn = true; 
          }
        }
        if (myturn)
        {
          if (lastMove.length() > 1){
            DEBUG_SERIAL.print("opponents move: ");
            DEBUG_SERIAL.println(lastMove);
            DEBUG_SERIAL.print("wait for move accept...");
            String accept_move;      
            while(accept_move != lastMove && is_game_running){
              displayMove(lastMove);
              accept_move = getMoveInput();
            }
          }
          DEBUG_SERIAL.print("wait for move input...");   
          postMove(PostClient);
        }
      }
    }
  }
}
