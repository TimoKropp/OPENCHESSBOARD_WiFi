#include "openchessboard.h"
/* ---------------------------------------
 *  Function to send post move request to Lichess API.
 *  Restarts client and stops client after request 
 *  @params[in] WiFiClientSecure
 *  @return void
*/
void postMove(WiFiClientSecure  &client) {

  while (myturn && is_game_running)
          {
            clearDisplay();
            DEBUG_SERIAL.print("wait for move input...");
            
            String move_input = getMoveInput();
            clearDisplay();
            DEBUG_SERIAL.print("my move: ");
            DEBUG_SERIAL.println(move_input);
            disableISR();
            myMove = move_input;
            if(is_game_running){
              if (!client.connected()) {
                  client.connect(server, 443);
              }
            }
            else{
              break;
            }
            client.print("POST /api/board/game/");
            client.print((String)currentGameID);
            client.print("/move/");
            client.print(move_input);
            client.println(" HTTP/1.1");
            client.println("Host: lichess.org");
            client.print("Authorization: Bearer ");
            client.println(lichess_api_token);
            client.println("Connection: close");
            client.println("\n");
            delay(100);
            char* char_response = catchResponseFromClient(client);
            //DEBUG_SERIAL.println(char_response);
            String moveSuccess = parseValueFromResponse(char_response, "ok");        
            client.stop();
            
            if (moveSuccess == "true") {
              DEBUG_SERIAL.println("move success!");
              myturn = false;
              client.connect(server, 443);
              enableISR();
            }
            else
            {        
                DEBUG_SERIAL.println("wrong move!");       
                displayMove(myMove);
                String reverse_move =  (String)myMove.charAt(2) 
                +  (String)myMove.charAt(3)
                +  (String)myMove.charAt(0)
                +  (String)myMove.charAt(1);
                
                DEBUG_SERIAL.println(reverse_move);   
                
                while(reverse_move != move_input && is_game_running){
                  move_input = getMoveInput();
                  DEBUG_SERIAL.println(move_input);  
                }
            }
          }
      
}

/* ---------------------------------------
 *  Function to send get stream request to Lichess API.
 *  Starts the move stream on client.
 *  @params[in] WiFiClientSecure
 *  @return void
*/  
void getStream(WiFiClientSecure  &client){
    client.print("GET /api/board/game/stream/");
    client.print((String)currentGameID);
    client.println(" HTTP/1.1");
    client.println("Host: lichess.org");
    client.print("Authorization: Bearer ");
    client.println(lichess_api_token);
    client.println("Connection: keep-alive");
    client.println("\n");
  } 

void disableClient(WiFiClientSecure  &client){
    client.println("GET /api/account/playing HTTP/1.1");
    client.println("Host: lichess.org");
    client.print("Authorization: Bearer ");
    client.println(lichess_api_token);
    client.println("Connection: close");
    client.println("\n"); 
    char* char_response = catchResponseFromClient(client);
    client.flush();
    client.stop();
  } 

/* ---------------------------------------
 *  Function to send get gameID request to Lichess API.
 *  If game is found, global variable currentGameID is set  and sets turn global variable myturn
 *  @params[in] WiFiClientSecure
 *  @return void
*/       
void getGameID(WiFiClientSecure  &client){
    // Request setup
    client.println("GET /api/account/playing HTTP/1.1");
    client.println("Host: lichess.org");
    client.print("Authorization: Bearer ");
    client.println(lichess_api_token);
    client.println("Connection: keep-alive");
    client.println("\n"); 
    delay(300);
    char* char_response = catchResponseFromClient(client);
    //DEBUG_SERIAL.print(char_response);
    currentGameID = parseValueFromResponse(char_response, "gameId");
    DEBUG_SERIAL.print("current game id: ");
    DEBUG_SERIAL.println(currentGameID);

    if (currentGameID.length() == 8){
        is_seeking = false;
        is_game_running = true;
    }
    myturn = parseValueFromResponse(char_response, "isMyTurn");
    DEBUG_SERIAL.print("my turn: ");
    DEBUG_SERIAL.println(myturn);

    lastMove = parseValueFromResponse(char_response, "lastMove");
    DEBUG_SERIAL.print("last move: ");
    DEBUG_SERIAL.println(lastMove);
}

char* catchResponseFromClient(WiFiClientSecure &client) {
    static char char_response[2024] = {0}; 

    while (!client.available()) {
        delay(1);
    }

    size_t length = 0;
    while (client.available() && length < sizeof(char_response) - 1) {
        char c = client.read();
        char_response[length++] = c;
    }
    char_response[length] = '\0'; 
    return char_response; 
}


String parseValueFromResponse(const char* response, const char* key) {
    static char valueBuffer[256]; 
    // Clear the buffer before use
    memset(valueBuffer, 0, sizeof(valueBuffer));

    String strResponse = String(response); 

    int jsonStart = strResponse.indexOf('{');
    if (jsonStart == -1) {
        DEBUG_SERIAL.println("JSON start not found.");
        return "no"; // Return NULL if no JSON object is found
    }
    strResponse = strResponse.substring(jsonStart); 

    String keyString = "\"" + String(key) + "\":";
    int keyStart = strResponse.indexOf(keyString);
    if (keyStart == -1) {
        DEBUG_SERIAL.print(key);
        DEBUG_SERIAL.println(" not found.");
        return "no"; 
    }
    keyStart += keyString.length(); 

    int valueEnd = strResponse.indexOf(',', keyStart);
    if (valueEnd == -1) {
        valueEnd = strResponse.indexOf('}', keyStart);
    }
    if (valueEnd == -1) {
        valueEnd = strResponse.length(); 
    }

    String value = strResponse.substring(keyStart, valueEnd);

    value.trim(); 

    if (value.startsWith("\"")) {
        value = value.substring(1);
    }
    if (value.endsWith("\"") || value.endsWith("}")) {
        value = value.substring(0, value.length() - 1);
    }

    value.toCharArray(valueBuffer, sizeof(valueBuffer));

    return value; 
}

void postNewGame(WiFiClientSecure &client, String board_gameMode) {
    String rated_str = "false";
    String variant = "standard";
    String ratingRange = "";  // Optional
    String days_str = "";     // Default for correspondence games
    String initialStr;
    String incrementStr;
    String endpoint;
    String requestBody;
    
    if (board_gameMode.startsWith("AI level ")) {
        // AI Challenge Mode
        int level = board_gameMode.substring(9).toInt();  // Extract and convert to integer
        if (level < 1 || level > 8) {
            DEBUG_SERIAL.println("Invalid AI level.");
            return;
        }
        
        endpoint = "/api/challenge/ai";  // API endpoint
        requestBody = "level=" + String(level) + "&rated=" + rated_str + "&variant=" + variant; // Correct format

        DEBUG_SERIAL.print("AI Challenge Endpoint: ");
        DEBUG_SERIAL.println(endpoint);
        DEBUG_SERIAL.print("Request Body: ");
        DEBUG_SERIAL.println(requestBody);
    } else {
        // Human Seek Mode
        int plusPos = board_gameMode.indexOf('+');
        if (plusPos != -1) {
            initialStr = board_gameMode.substring(0, plusPos);
            incrementStr = board_gameMode.substring(plusPos + 1);
        } else {
            DEBUG_SERIAL.println("Invalid time control format.");
            return;
        }

        endpoint = "/api/board/seek";  // Correct API endpoint
        requestBody = "rated=" + rated_str + 
                      "&time=" + initialStr + 
                      "&increment=" + incrementStr + 
                      "&days=" + days_str +  
                      "&variant=" + variant + 
                      "&ratingRange=" + ratingRange;
        
        DEBUG_SERIAL.print("Human Seek Endpoint: ");
        DEBUG_SERIAL.println(endpoint);
        DEBUG_SERIAL.print("Request Body: ");
        DEBUG_SERIAL.println(requestBody);
    }
  
    // Ensure client is connected
    if (!client.connected()) {
        client.connect("lichess.org", 443);
    }
    
    // Send the POST request
    client.print("POST " + endpoint + " HTTP/1.1\r\n");
    client.println("Host: lichess.org");
    client.print("Authorization: Bearer ");
    client.println(lichess_api_token);
    client.println("Content-Type: application/x-www-form-urlencoded");  // Required for form data
    client.print("Content-Length: ");
    client.println(requestBody.length());  // Ensure correct content length
    client.println("Connection: close\r\n");  // Blank line before body
    client.println(requestBody);  // Send body
    
    delay(300);
    char* char_response = catchResponseFromClient(client);
    DEBUG_SERIAL.println(char_response);

}
