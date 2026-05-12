#include "openchessboard.h"
#include <Logger.h>
/* ---------------------------------------
 *  Function to send post move request to Lichess API.
 *  Restarts client and stops client after request 
 *  @params[in] WiFiClientSecure
 *  @return void
*/
bool postMove(WiFiClientSecure  &client, String move) {
            if (!client.connected()) {
                client.connect(server, 443);
            }
            client.print("POST /api/board/game/");
            client.print((String)currentGameID);
            client.print("/move/");
            client.print(move);
            client.println(" HTTP/1.1");
            client.println("Host: lichess.org");
            client.print("Authorization: Bearer ");
            client.println(lichess_api_token);
            client.println("Connection: close");
            client.println("\n");
            delay(200);
            char* char_response = catchResponseFromClient(client);
            client.stop();
            LOG_DEBUG << "Response: " << char_response;
            JsonDocument doc;

            bool moveSuccess = false;
            if (parseJsonResponse(char_response, doc)) {
                moveSuccess = doc["ok"].as<bool>();
            }
            if (moveSuccess){
                LOG_INFO << "Move accepted";
            }
    return moveSuccess;
}

/* ---------------------------------------
 *  Function to send get stream request to Lichess API.
 *  Starts the move stream on client.
 *  @params[in] WiFiClientSecure
 *  @return void
*/  
void getStream(WiFiClientSecure  &client){
    if (!client.connected()) {
        client.connect(server, 443);
    }
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
    if (!client.connected()) {
        client.connect(server, 443);
    }
    client.println("GET /api/account/playing HTTP/1.1");
    client.println("Host: lichess.org");
    client.print("Authorization: Bearer ");
    client.println(lichess_api_token);
    client.println("Connection: close");
    client.println("\n"); 

    char* char_response = catchResponseFromClient(client);
    LOG_DEBUG << "Response: " << char_response;

    JsonDocument doc;

    if (parseJsonResponse(char_response, doc)) {
        currentGameID = doc["nowPlaying"][0]["gameId"].as<String>();   
    }

    if (currentGameID.length() == 8){  
        LOG_INFO << "Current game ID: " << currentGameID;

        bool myturn_temp  = doc["nowPlaying"][0]["isMyTurn"].as<bool>();
        LOG_INFO << "My Turn: " << myturn_temp;

        if(myturn_temp){
            myturn = true;
        }

        String lastMove_temp  = doc["nowPlaying"][0]["lastMove"].as<String>();
        LOG_INFO << "Last move: " << lastMove_temp;

        if(lastMove_temp.length() == 4  & myturn){ //  last move was played by opponent and its my turn
            oppLastMove = lastMove_temp;
            latestMove = lastMove_temp;
            moves = lastMove_temp; // complete  board history not yet available
        }
        if(lastMove_temp.length() == 4  & !myturn){ // last move was played by player and wait for opponent move 
            myLastMove = lastMove_temp;
            latestMove = lastMove_temp;
            moves = lastMove_temp; // complete  board history not yet available
        }
        setStatePlaying();
        client.flush();
        client.stop();     
    }
    else{
        currentGameID = "noGame";
        LOG_INFO << "No game found";
        delay(300);
    }
    
}

char* catchResponseFromClient(WiFiClientSecure &client) {
    static char char_response[2024] = {0}; 

    unsigned long startTime = millis();

    while (!client.available()) {
        if (millis() - startTime > 800) {
            char_response[0] = '\0'; 
            return char_response;
        }
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

bool parseJsonResponse(const char* response, JsonDocument& doc) {
    const char* jsonStart = strchr(response, '{');
    if (!jsonStart) {
        LOG_DEBUG << "No JSON object found in response!";
        return false;
    }

    DeserializationError error = deserializeJson(doc, jsonStart);
    if (error) {
        LOG_INFO << "JSON parse failed: " << error;
        return false;
    }

    return true;
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
            LOG_INFO << "Invalid AI level.";
            return;
        }
        
        endpoint = "/api/challenge/ai";  // API endpoint
        requestBody = "level=" + String(level) + "&rated=" + rated_str + "&variant=" + variant; // Correct format

        LOG_INFO << "AI Challenge Endpoint: " << endpoint;
        LOG_DEBUG << "Request Body: " << requestBody;
    } else {
        // Human Seek Mode
        int plusPos = board_gameMode.indexOf('+');
        if (plusPos != -1) {
            initialStr = board_gameMode.substring(0, plusPos);
            incrementStr = board_gameMode.substring(plusPos + 1);
        } else {
            LOG_INFO << "Invalid time control format.";
            return;
        }

        endpoint = "/api/board/seek";  // Correct API endpoint
        requestBody = "rated=" + rated_str + 
                      "&time=" + initialStr + 
                      "&increment=" + incrementStr + 
                      "&days=" + days_str +  
                      "&variant=" + variant + 
                      "&ratingRange=" + ratingRange;
        
        LOG_INFO << "Human Seek Endpoint: " << endpoint;
        LOG_DEBUG << "Request Body: " << requestBody;
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

    char* char_response = catchResponseFromClient(client);
    is_seeking = true;
    LOG_DEBUG << "Response: " << char_response;
    client.flush();
    client.stop();
}
