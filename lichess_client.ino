void postMove() {
          while (myturn && is_game_running)
          {
            clearDisplay();
            String move_input = getMoveInput();
            clearDisplay();
            DEBUG_SERIAL.print("my move: ");
            DEBUG_SERIAL.println(move_input);
          
            myMove = move_input;
            client2.connect(server, 443);
  
            client2.print("POST /api/board/game/");
            client2.print((String)currentGameID);
            client2.print("/move/");
            client2.print(move_input);
            client2.println(" HTTP/1.0");
            client2.println("Host: lichess.org");
            client2.print("Authorization: Bearer ");
            client2.println(token);
            client2.println("Connection: keep-alive");
            client2.println();
            delay(300);
            boolean moveSuccess = false;
            processHTTP2(); // if the move is invalid it will get picked up in this function as a 400 Bad request
            StaticJsonDocument<48> doc;
            DeserializationError error = deserializeJson(doc, client2);
  
            client2.stop();
            //determine whether the move was successful
            moveSuccess = doc["ok"];
            if (moveSuccess == true) {
              DEBUG_SERIAL.println("move success!");
              myturn = false;
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

void getUsername(){
     DEBUG_SERIAL.println("connected to server in setup");
    // SETUP API: MAKE A REQUEST TO DOWNLOAD THE CURRENT USER'S LICHESS USERNAME
    client.println("GET /api/account HTTP/1.1");
    client.println("Host: lichess.org");
    // Include an authorisation header with the lichess API token
    client.print("Authorization: Bearer ");
    client.println(token);
    delay(100); 
    //process the request and parse the header
    processHTTP();
    // Allocate the JSON document
    // Use arduinojson.org/v6/assistant to compute the capacity.
    DynamicJsonDocument doc(2048);
    // Parse JSON object
    DeserializationError error = deserializeJson(doc, client);
    if (error)
    {
      // this is due to an error in the HTTP request
      DEBUG_SERIAL.print(F("deserializeJson() failed: "));
      DEBUG_SERIAL.println(error.f_str());
      return;
    }
    // Extract values
    DEBUG_SERIAL.println(F("Connected to User:"));
    // lichess username
    username = doc["username"];
    DEBUG_SERIAL.println(username);
    //close request
    client.println("Connection: close");
    client.println();
    if (username != NULL) {
      delay(100);
    }
  }
  
void getTurn(){

    client.println("GET /api/account/playing HTTP/1.1");
    client.println("Host: lichess.org");
    client.print("Authorization: Bearer ");
    client.println(token);
    delay(100);  
    processHTTP();

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, client);
    
    if (error)
    {
      DEBUG_SERIAL.print(F("deserializeJson() failed: "));
      DEBUG_SERIAL.println(error.f_str());
    }

    // Extract Game ID
    JsonObject nowPlaying_0 = doc["nowPlaying"][0];
    myturn = nowPlaying_0["isMyTurn"];
  } 
   
void getGameID(){
  
    DEBUG_SERIAL.println("Find ongoing game");

    //keep the request so lichess knows you are there
    client.println("GET /api/account/playing HTTP/1.1");
    client.println("Host: lichess.org");
    client.print("Authorization: Bearer ");
    client.println(token);
    delay(100); //delay to allow a response
    processHTTP();

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, client);
    if (error)
    {
      DEBUG_SERIAL.print(F("deserializeJson() failed: "));
      DEBUG_SERIAL.println(error.f_str());
      return;
    }

    // Extract Game ID
    JsonObject nowPlaying_0 = doc["nowPlaying"][0];
    JsonObject nowPlaying_0_opponent = nowPlaying_0["opponent"];
    currentGameID = nowPlaying_0["gameId"];
    myturn = nowPlaying_0["isMyTurn"];

    DEBUG_SERIAL.println(currentGameID);
}
  
void processHTTP() {

  // this function processes http data before it is read by arduino json
  if (client.println() == 0) {
    return;
  }
  // Check HTTP status
  char status[64] = {0};
  client.readBytesUntil('\r', status, sizeof(status));

  // It should be "HTTP/1.0 200 OK"
  if (strcmp(status + 9, "200 OK") != 0) {
    //DEBUG_SERIAL.print(F("Unexpected response: "));
    //DEBUG_SERIAL.println(status);
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
    //DEBUG_SERIAL.println(F("Invalid response"));
    return;
  }
}

void processHTTP2() {

  // this function processes http data before it is read by arduino json
  if (client2.println() == 0) {
    //DEBUG_SERIAL.println(F("Failed to send request"));
    return;
  }
  // Check HTTP status
  char status[64] = {0};
  client2.readBytesUntil('\r', status, sizeof(status));


  // It should be "HTTP/1.0 200 OK"
  if (strcmp(status + 9, "200 OK") != 0) {
    //DEBUG_SERIAL.print(F("Unexpected response: "));
    //DEBUG_SERIAL.println(status);
    if (strcmp(status + 9, "400 Bad Request") == 0) {
      //delay(2000);
    }
    else {
      return;
    }
  }
  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client2.find(endOfHeaders)) {
    //DEBUG_SERIAL.println(F("Invalid response"));
    return;
  }
}
