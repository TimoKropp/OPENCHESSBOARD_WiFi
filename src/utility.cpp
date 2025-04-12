#include "openchessboard.h"
/* ---------------------------------------
 *  function to get substring between string firstdel and enddel
 *  @params[in] void
 *  @return substring
*/
String GetStringBetweenStrings(String input, String firstdel, String enddel) {
  int posfrom = input.indexOf(firstdel) + firstdel.length();
  int posto   = input.indexOf(enddel);
  return input.substring(posfrom, posto);
}

/* ---------------------------------------
 *  function to check if move is part of castling move and wait for rook move to complete castling
 *  @params[in] String move_input
 *  @return void
*/
void checkCastling(String move_input) {
  //check if last move was king move from castling
  if(((move_input == "e1g1") || (move_input == "e1c1") ||  (move_input == "e8g8") || (move_input == "e8c8")) && is_castling_allowed){
    
   DEBUG_SERIAL.println("Castling... Wait for rook move...");
   
   bool is_castling = true;
   
   //wait until move was rook move from castling
   while(is_castling && is_game_running){
    displayMove(myLastMove);
    move_input = getMoveInput();
    DEBUG_SERIAL.println(move_input);
    
    if((move_input == "h1f1") || (move_input == "a1d1") ||  (move_input == "h8f8") || (move_input == "a8d8"))
    {
      is_castling = false;
      is_castling_allowed = false;
    }
  
   }
  }  
}


/* ---------------------------------------
 *  function to set booting state and initializes state variables
 *  @params[in] void
 *  @return void
*/
void setStateBooting(void){
  is_game_running = false;
  is_booting = true;
  is_connecting = false;
  is_updating = false;
  is_seeking = false;   
  myLastMove = "xx";
  oppLastMove = "xy";
  latestMove = "zz";
  myMove = "noMove";
  moves = "noMoves";
  currentGameID = "noGameID";
  //myturn = false;
}

void setStateUpdating(void){
  is_game_running = false;
  is_booting = false;
  is_connecting = false;
  is_updating = true;
  is_seeking = false;   
  myLastMove = "xx";
  oppLastMove = "xy";
  latestMove = "zz";
  myMove = "yy";
  moves = "";
  currentGameID = "noGameID";
  //myturn = false;
}
/* ---------------------------------------
 *  function to set connecting state and initializes state variables
 *  @params[in] void
 *  @return void
*/
void setStateConnecting(void){
  is_game_running = false;
  is_booting = false;
  is_connecting  = true;
  is_updating = false;
  is_seeking = false;   
  myLastMove = "xx";
  oppLastMove = "xy";
  latestMove = "zz";
  myMove = "yy";
  moves = "";
  currentGameID = "noGameID";
  //myturn = false;
}

void setStatePlaying(void){
  is_seeking = false;    
  is_game_running = true;
  is_updating = false;
  is_connecting = false;
  displayNewGame();
}

String urlDecode(const String &encoded) {
  String decoded = "";
  char ch;
  int i = 0;
  while (i < encoded.length()) {
    ch = encoded.charAt(i);
    if (ch == '%') {
      // Get the next two characters after %
      String hexCode = encoded.substring(i + 1, i + 3);
      decoded += (char) strtol(hexCode.c_str(), NULL, 16);
      i += 3;
    } else if (ch == '+') {
      // Decode the + as a space
      decoded += ' ';
      i++;
    } else {
      decoded += ch;
      i++;
    }
  }
  return decoded;
}

void readSettings(void){

  preferences.begin("settings", true);

  if (preferences.isKey("ssid")) {
    wifi_ssid = urlDecode(preferences.getString("ssid", ""));
    wifi_password = urlDecode(preferences.getString("password", ""));
    lichess_api_token = urlDecode(preferences.getString("token", ""));
    board_gameMode = urlDecode(preferences.getString("gameMode", ""));
    board_startupType = preferences.getString("startupType", "");
    DEBUG_SERIAL.println("Settings Loaded from Flash:");
    DEBUG_SERIAL.println("SSID: " + wifi_ssid);
    DEBUG_SERIAL.println("Password: " + wifi_password);
    DEBUG_SERIAL.println("Token: " + lichess_api_token);
    DEBUG_SERIAL.println("Game Mode: " + board_gameMode);
    DEBUG_SERIAL.println("Startup Type: " + board_startupType);
  } else {
    DEBUG_SERIAL.println("No settings found, using default values.");
  }
  preferences.end();
}

bool isStartingPosition(void){
  byte read_hall_array[8];
  byte diff_pattern[8];
  byte pattern1[8];
  memset(pattern1, 0xC3, sizeof(pattern1));
  readHall(read_hall_array);

  calculateDifference(diff_pattern, read_hall_array, pattern1);
  rotate180(diff_pattern);
  dimLEDs = true;
  displayArray(diff_pattern);

  if (memcmp(read_hall_array, pattern1, 8) == 0){
    return true;
  }
  else{
    return false;
  }
}

void readBoardSelection(){
  byte read_hall_array[8];
  byte pattern1[8];
  byte pattern2[8];
  byte pattern3[8];
  byte pattern4[8];

  memset(pattern1, 0xC3, sizeof(pattern1));
  memset(pattern2, 0xC3, sizeof(pattern2));
  memset(pattern3, 0xC3, sizeof(pattern3));
  memset(pattern4, 0x00, sizeof(pattern4));

  pattern1[7] =0xC2; // remove piece on a1 to select this
  pattern2[6] =0xC2; // remove piece on b1 to select this
  pattern3[5] =0xC2; // remove piece on b1 to select this

  pattern4[0] =0xFF; // place 8 pieces on h column (plug at right)
  

  readHall(read_hall_array);
  DEBUG_SERIAL.print("read_hall_array: ");

  for (int i = 0; i < 8; i++) {
      Serial.print(read_hall_array[i], HEX);
      if (i < 8 - 1) {
          DEBUG_SERIAL.print(", "); 
      }
  }
    DEBUG_SERIAL.println(); // Newline at the end
  if (memcmp(read_hall_array, pattern1, 8) == 0){
    board_startupType = "WiFi";
  }
  else if (memcmp(read_hall_array, pattern2, 8) == 0)
  {
    board_startupType = "BLE";
  }
  else if (memcmp(read_hall_array, pattern3, 8) == 0)
  {
    board_startupType = "AP";
  }
  else if (memcmp(read_hall_array, pattern4, 8) == 0)
  {
    board_startupType = "PUZZLE";
  }
}