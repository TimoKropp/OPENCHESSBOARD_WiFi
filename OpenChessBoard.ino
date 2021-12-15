//Includes
#include <WiFiNINA.h>
#include <ArduinoJson.h>

//HW variables
int LED_MR_N_PIN = 9; // RESET, D9
int LED_CLOCK_PIN = 8; //SHCP, D8
int LED_LATCH_PIN = 7; //STCP, D7
int LED_OE_N_PIN = 6; // D6
int LED_DATA_PIN = 11; //D1

int HALL_OUT_S0 = 14; //D14
int HALL_OUT_S1 = 13; //D13
int HALL_OUT_S2 = 12; //D12

int HALL_ROW_S0 = 5;  //D5 
int HALL_ROW_S1 = 4;  //D4
int HALL_ROW_S2 = 3;  //D3

int HALL_SENSE = A1;
int sense_val = 0;


// WiFi variables
int status = WL_IDLE_STATUS;
char server[] = "lichess.org";  // name address for lichess (using DNS)
WiFiSSLClient client; // WIFISSLClient for move stream, always connects via SSL (port 443 for https)
WiFiSSLClient client2; // WIFISSLClient for post moves, always connects via SSL (port 443 for https)


//Secret data
//char ssid[] = "Oioioi2.4";     // your network SSID (name)
//char pass[] = "1208salat!";    // your network password (use for WPA, or use as key for WEP)
char ssid[] = "Board";     // your network SSID (name)
char pass[] = "board123";    // your network password (use for WPA, or use as key for WEP)
char token[] = "wF6HUwZfyJgbvPqF"; // your lichess API token : Arduino_Lichess, botmokko2


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
  
  if (client.connect(server, 443))
  {
    getUsername();
  }
  else {
    DEBUG_SERIAL.print("no connection to server");
  }
  
}

void loop() {

  is_booting = false;
  is_connecting = true;
  
  if (client.connect(server, 443))
  {
    getGameID();
    
    if (currentGameID != NULL)
    {
      is_game_running = true;
      is_connecting = false;
      
      //TC4->COUNT16.CTRLA.bit.ENABLE = 0; // disable interupt to stop connection blink animation
      
      DEBUG_SERIAL.println("Wait for next move");
      

      client.print("GET /api/board/game/stream/");
      client.print((String)currentGameID);
      client.println(" HTTP/1.1");
      client.println("Host: lichess.org");
      client.print("Authorization: Bearer ");
      client.println(token);
      client.println("Connection: close");
      client.println();   
      
      DEBUG_SERIAL.println("wait for incoming moves");
      

      while (is_game_running)
      {
       while (!myturn && is_game_running){
         
         char char_response[800] = {0};
         
         while(client.available()) {
          char_response[800] = {0};
          client.readBytesUntil('\n', char_response, sizeof(char_response));
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
          postMove();
        }
      }
    }
  }
}
