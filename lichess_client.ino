/* ---------------------------------------
 *  Function to send post move request to Lichess API.
 *  Restarts client and stops client after request 
 *  @params[in] WiFiSSLClient
 *  @return void
*/
void postMove(WiFiSSLClient &client) {
          while (myturn && is_game_running)
          {
            clearDisplay();
            String move_input = getMoveInput();
            clearDisplay();
            DEBUG_SERIAL.print("my move: ");
            DEBUG_SERIAL.println(move_input);
          
            myMove = move_input;
            
            TC4->COUNT16.CTRLA.bit.ENABLE = 0;

            if(!client.connected()){
              client.connect(server, 443); 
            }

            client.print("POST /api/board/game/");
            client.print((String)currentGameID);
            client.print("/move/");
            client.print(move_input);
            client.println(" HTTP/1.1");
            client.println("Host: lichess.org");
            client.print("Authorization: Bearer ");
            client.println(token);
            client.println("Connection: close");
            client.println();
            
            delay(500);

            processHTTP(client);

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, client);

            client.stop();
              
            //check for sucessful move
            boolean moveSuccess = false;
            moveSuccess = doc["ok"];
            if (moveSuccess == true) {
              DEBUG_SERIAL.println("move success!");
              myturn = false;
              client.connect(server, 443);
              TC4->COUNT16.CTRLA.bit.ENABLE = 1; 
            }
            else
            {    
                TC4->COUNT16.CTRLA.bit.ENABLE = 1;             
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
 *  Function to send get username move request to Lichess API.
 *  Sets requested username to global variable username.
 *  @params[in] WiFiSSLClient
 *  @return void
*/
void getUsername(WiFiSSLClient &client){
     DEBUG_SERIAL.println("connected to server in setup");
    // SETUP API: MAKE A REQUEST TO DOWNLOAD THE CURRENT USER'S LICHESS USERNAME
    client.println("GET /api/account HTTP/1.1");
    client.println("Host: lichess.org");
    // Include an authorisation header with the lichess API token
    client.print("Authorization: Bearer ");
    client.println(token);
    delay(100); 

    processHTTP(client);

    DynamicJsonDocument doc(1536);
    DeserializationError error = deserializeJson(doc, client);
    if (error)
    {
      DEBUG_SERIAL.print(F("deserializeJson() failed: "));
      DEBUG_SERIAL.println(error.f_str());
      return;
    }
    DEBUG_SERIAL.println(F("Connected to User:"));

    username = doc["username"];
    DEBUG_SERIAL.println(username);
    
    //close request
    client.println("Connection: close");
    client.println();
    if (username != NULL) {
      delay(100); // slow down to not spam requests
    }
  }

/* ---------------------------------------
 *  Function to send get stream request to Lichess API.
 *  Starts the move stream on client.
 *  @params[in] WiFiSSLClient
 *  @return void
*/  
void getStream(WiFiSSLClient &client){
    client.print("GET /api/board/game/stream/");
    client.print((String)currentGameID);
    client.println(" HTTP/1.1");
    client.println("Host: lichess.org");
    client.print("Authorization: Bearer ");
    client.println(token);
    client.println("Connection: close");
    client.println();
    delay(500);  
    processHTTP(client);
  } 


/* ---------------------------------------
 *  Function to send get gameID request to Lichess API.
 *  If game is found, global variable currentGameID is set  and sets turn global variable myturn
 *  @params[in] WiFiSSLClient
 *  @return void
*/       
void getGameID(WiFiSSLClient &client){
    client.println("GET /api/account/playing HTTP/1.1");
    client.println("Host: lichess.org");
    client.print("Authorization: Bearer ");
    client.println(token);
    delay(100); 
    
    processHTTP(client);

    DynamicJsonDocument doc(1536);
    DeserializationError error = deserializeJson(doc, client);
    if (error)
    {
      DEBUG_SERIAL.print(F("deserializeJson() failed: "));
      DEBUG_SERIAL.println(error.f_str());
      return;
    }

    JsonObject nowPlaying_0 = doc["nowPlaying"][0];
    JsonObject nowPlaying_0_opponent = nowPlaying_0["opponent"];
    currentGameID = nowPlaying_0["gameId"];
    myturn = nowPlaying_0["isMyTurn"];
    
    DEBUG_SERIAL.println(currentGameID);
}


/* ---------------------------------------
 *  Function to evaluate http requests on wifi client.
 *  Generic function that checks for http status and skips headers
 *  @params[in] WiFiSSLClient
 *  @return void
*/   
void processHTTP(WiFiSSLClient client) {
  if (client.println() == 0) {
    return;
  }

  char status[64] = {0};
  client.readBytesUntil('\r', status, sizeof(status));

  // It should be "HTTP/1.0 200 OK"
  if (strcmp(status + 9, "200 OK") != 0) {
    DEBUG_SERIAL.print(F("Unexpected response: "));
    DEBUG_SERIAL.println(status);
    if (strcmp(status + 9, "400 Bad Request") == 0) {
      //catch bad request
    }
    else {
      return;
    }
  }
  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    DEBUG_SERIAL.println(F("Invalid response"));
    return;
  }
}
