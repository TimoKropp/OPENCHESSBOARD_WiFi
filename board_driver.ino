//HW GPIO configuration

int LED_MR_N_PIN = 4; // RESET, D4
int LED_CLOCK_PIN = 6; //SHCP, D5
int LED_LATCH_PIN = 5; //STCP, D6 
int LED_OE_N_PIN = 3; // D3 
int LED_DATA_PIN = 2; //D2

int HALL_OUT_S0 = 10; //D10
int HALL_OUT_S1 = 9; //D9
int HALL_OUT_S2 = 8; //D8

int HALL_ROW_S0 = A7;  //A7/D21
int HALL_ROW_S1 = A6;  //A6/D20
int HALL_ROW_S2 = A5;  //A5/D19

int HALL_SENSE = A3;  //A3

#define LED_BRIGHTNESS 100 // LED brightess in percent

#define LED_DIM_VAL (255 - LED_BRIGHTNESS/100 * 255)  
#define LED_OFF_VAL 255

/* ---------------------------------------
 *  Function to initiate GPIOs.
 *  Defines GPIOs input and output states. 
 *  Depends on Arduino HW (adapt HW GPIO configuration to match Arduino Board)
 *  @params[in] void
 *  @return void
*/   
void initHW(void) {
  pinMode(LED_MR_N_PIN, OUTPUT);
  pinMode(LED_OE_N_PIN, OUTPUT);
  digitalWrite(LED_MR_N_PIN, 0);
  digitalWrite(LED_OE_N_PIN, 1);
  
  pinMode(LED_CLOCK_PIN, OUTPUT);
  pinMode(LED_LATCH_PIN, OUTPUT);
  pinMode(LED_DATA_PIN, OUTPUT);



  pinMode(HALL_OUT_S0, OUTPUT);
  pinMode(HALL_OUT_S1, OUTPUT);
  pinMode(HALL_OUT_S2, OUTPUT);

  pinMode(HALL_ROW_S0, OUTPUT);
  pinMode(HALL_ROW_S1, OUTPUT);
  pinMode(HALL_ROW_S2, OUTPUT);

  pinMode(HALL_SENSE, INPUT);

}


/* ---------------------------------------
 *  Function to write array to LED shift registers.
 *  Activates LEDs immediately.
 *  @params[in] byte array (max size 8 bytes)
 *  @return void
*/   
void shiftOut(byte led_data_array[]) {

  bool pinStateEN;

  digitalWrite(LED_DATA_PIN, 0);
  digitalWrite(LED_MR_N_PIN, 0);
  delay(1);
  digitalWrite(LED_MR_N_PIN, 1);

  for (int i = 0; i < 8; i++)  {
    for (int k = 0; k < 8; k++) {

      digitalWrite(LED_CLOCK_PIN, 0);

      if (led_data_array[i] & (1 << k)) {
        pinStateEN = 1;
      }
      else {
        pinStateEN = 0;
      }
      
      digitalWrite(LED_DATA_PIN, pinStateEN);
      digitalWrite(LED_CLOCK_PIN, 1);
      digitalWrite(LED_DATA_PIN, 0);
    }
  }
  digitalWrite(LED_CLOCK_PIN, 0);

}


/* ---------------------------------------
 *  Function to ready Hall sensors states to array.
 *  Multiplexing all sensors. Sets 0 or 1 n array if threshold is exceeded.
 *  @params[in] byte array (max size 8 bytes)
 *  @return void
*/   
void readHall(byte read_hall_array[]) {
  int row_index = 0;
  int col_index = 0;
  int sense_val = 0;
  int sense_thrs = 200;
  
  for (int k = 0; k < 8; k++) {
    read_hall_array[k] = 0x00;
  }

  for (row_index = 0; row_index < 8; row_index++)
  {
    switch (row_index) {
      case 0:
        digitalWrite(HALL_ROW_S0, 0);
        digitalWrite(HALL_ROW_S1, 0);
        digitalWrite(HALL_ROW_S2, 0);
        break;
      case 1:
        digitalWrite(HALL_ROW_S0, 1);
        digitalWrite(HALL_ROW_S1, 0);
        digitalWrite(HALL_ROW_S2, 0);
        break;
      case 2:
        digitalWrite(HALL_ROW_S0, 0);
        digitalWrite(HALL_ROW_S1, 1);
        digitalWrite(HALL_ROW_S2, 0);
        break;
      case 3:
        digitalWrite(HALL_ROW_S0, 1);
        digitalWrite(HALL_ROW_S1, 1);
        digitalWrite(HALL_ROW_S2, 0);
        break;
      case 4:
        digitalWrite(HALL_ROW_S0, 0);
        digitalWrite(HALL_ROW_S1, 0);
        digitalWrite(HALL_ROW_S2, 1);
        break;
      case 5:
        digitalWrite(HALL_ROW_S0, 1);
        digitalWrite(HALL_ROW_S1, 0);
        digitalWrite(HALL_ROW_S2, 1);
        break;
      case 6:
        digitalWrite(HALL_ROW_S0, 0);
        digitalWrite(HALL_ROW_S1, 1);
        digitalWrite(HALL_ROW_S2, 1);
        break;
      case 7:
        digitalWrite(HALL_ROW_S0, 1);
        digitalWrite(HALL_ROW_S1, 1);
        digitalWrite(HALL_ROW_S2, 1);
        break;
    }
    for (col_index = 0; col_index < 8; col_index++) {
      switch (col_index) {
        case 0:
          digitalWrite(HALL_OUT_S0, 0);
          digitalWrite(HALL_OUT_S1, 0);
          digitalWrite(HALL_OUT_S2, 0);
          break;
        case 1:
          digitalWrite(HALL_OUT_S0, 1);
          digitalWrite(HALL_OUT_S1, 0);
          digitalWrite(HALL_OUT_S2, 0);
          break;
        case 2:
          digitalWrite(HALL_OUT_S0, 0);
          digitalWrite(HALL_OUT_S1, 1);
          digitalWrite(HALL_OUT_S2, 0);
          break;
        case 3:
          digitalWrite(HALL_OUT_S0, 1);
          digitalWrite(HALL_OUT_S1, 1);
          digitalWrite(HALL_OUT_S2, 0);
          break;
        case 4:
          digitalWrite(HALL_OUT_S0, 0);
          digitalWrite(HALL_OUT_S1, 0);
          digitalWrite(HALL_OUT_S2, 1);
          break;
        case 5:
          digitalWrite(HALL_OUT_S0, 1);
          digitalWrite(HALL_OUT_S1, 0);
          digitalWrite(HALL_OUT_S2, 1);
          break;
        case 6:
          digitalWrite(HALL_OUT_S0, 0);
          digitalWrite(HALL_OUT_S1, 1);
          digitalWrite(HALL_OUT_S2, 1);
          break;
        case 7:
          digitalWrite(HALL_OUT_S0, 1);
          digitalWrite(HALL_OUT_S1, 1);
          digitalWrite(HALL_OUT_S2, 1);
          break;
      }
      delayMicroseconds(10);
      sense_val = analogRead(HALL_SENSE);
      delayMicroseconds(100);
      if (sense_val < sense_thrs) {
        read_hall_array[row_index] |= 1UL << (col_index);
      }

    }
  }
}


/* ---------------------------------------
 *  Function that waits for a move input.
 *  Waits for a move input (blocking, but can be exited by isr if game is set to be not running) 
 *  and returns move string.
 *  Example move: e2e4(piece moves from e2 to e4)
 *  @params[in] void
 *  @return String move_input
*/  
String getMoveInput(void) {
  const char columns[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
  String move_input;
 
  byte piece_pos_array[8];
  byte cur_hall_array1[8];
  byte cur_hall_array2[8];
  byte cur_hall_array3[8];
  byte led_test_array[8];

  for (int k = 0; k < 8; k++) {
    piece_pos_array[k] = 0x00;
    cur_hall_array1[k] = 0x00;
    cur_hall_array2[k] = 0x00;
    cur_hall_array3[k] = 0x00;
    led_test_array[k] = 0x00;
  }

  bool is_move_started = false;
  bool is_piece_placed = false;
  bool is_move_finished = false;


  // get inital position
  readHall(piece_pos_array);

  while (!is_move_started && is_game_running) {
    readHall(cur_hall_array1);

    for (int row_index = 0; row_index < 8; row_index++) {
      for (int col_index = 0; col_index < 8; col_index++) {

        int state1 = bitRead(piece_pos_array[row_index], col_index);
        int state2 = bitRead(cur_hall_array1[row_index], col_index);
        if (state1  != state2) {
          led_test_array[7 - row_index] |= 1UL << (7 - col_index);
          move_input = move_input + (String)columns[7 - col_index] + (String)(7 - row_index + 1);
          is_move_started = true;
          break;
        }
      }
    }
  }

  Serial.println("piece is moving");
  digitalWrite(LED_LATCH_PIN, 0);
  shiftOut(led_test_array);
  digitalWrite(LED_LATCH_PIN, 1);
  digitalWrite(LED_OE_N_PIN , 0);
  //analogWrite(LED_OE_N_PIN , LED_DIM_VAL);

  while (!is_move_finished && is_game_running) {

    readHall(cur_hall_array2);
    delay(100);
    readHall(cur_hall_array3);
    delay(100);
    for (int row_index = 0; row_index < 8; row_index++) {
      for (int col_index = 0; col_index < 8; col_index++) {

        int state_prev = bitRead(cur_hall_array1[row_index], col_index);

        int state1 = bitRead(cur_hall_array2[row_index], col_index);
        int state2 = bitRead(cur_hall_array3[row_index], col_index);

        if ((state1 != state_prev) && (state2 != state_prev)) {
          if (state1  == state2) {
            is_move_finished = true;
            led_test_array[7 - row_index] |= 1UL << (7 - col_index);
            move_input = move_input + (String)columns[7 - col_index] + (String)(7 - row_index + 1);
          }
        }
      }
    }
  }
  digitalWrite(LED_LATCH_PIN, 0);
  shiftOut(led_test_array);
  digitalWrite(LED_LATCH_PIN, 1);
  digitalWrite(LED_OE_N_PIN , 0);
  //analogWrite(LED_OE_N_PIN , LED_DIM_VAL);
  delay(300);
  
  return move_input;
  
}


/* ---------------------------------------
 *  Function that clears all LED states.
 * Writes 0 to shift registers for all LEDs.
 *  @params[in] void
 *  @return void
*/  
void clearDisplay(void) {
  byte led_test_array[8];
  
  for (int k = 0; k < 8; k++) {
    led_test_array[k] = 0x00;
  }
  digitalWrite(LED_MR_N_PIN, 0);
  digitalWrite(LED_OE_N_PIN , 1);
  digitalWrite(LED_LATCH_PIN, 0);
  shiftOut(led_test_array);
  digitalWrite(LED_LATCH_PIN, 1);
  digitalWrite(LED_OE_N_PIN , 1);
  //analogWrite(LED_OE_N_PIN , LED_DIM_VAL);
}


/* ---------------------------------------
 *  Function that displays connection animation.
 *  Writes to specific shift registers and flips states periodically by isr.
 *  @params[in] void
 *  @return void
*/  
void displayConnectWait(void) {
  byte connect_led_array[8] = {0};

  if (connect_flipstate) {
    connect_led_array[3] = 0x10;
    connect_led_array[4] = 0x08;
  }
  else {
    connect_led_array[4] = 0x10;
    connect_led_array[3] = 0x08;
  }
  digitalWrite(LED_OE_N_PIN , 1);
  digitalWrite(LED_MR_N_PIN, 0);
  digitalWrite(LED_MR_N_PIN, 1);
  digitalWrite(LED_LATCH_PIN, 0);
  shiftOut(connect_led_array);
  digitalWrite(LED_LATCH_PIN, 1);
  digitalWrite(LED_OE_N_PIN , 0);
  //analogWrite(LED_OE_N_PIN , LED_DIM_VAL);
}


/* ---------------------------------------
 *  Function that transforms a move string to led array.
 *  Writes move input to led array.
 *  @params[in] byte array, string move
 *  @return void
*/  
void setDisplayMove(byte led_data_array[], String move_string) {

  const char columns[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
  const char rows[] = {'1', '2', '3', '4', '5', '6', '7', '8'};

  int row1 = 0;
  int col1 = 0;
  int row2 = 0;
  int col2 = 0;


  for (int k = 0; k < 8; k++)
  {
    if (columns[k] == move_string.charAt(0)) {
      col1 = k;
    }
    if (columns[k] == move_string.charAt(2)) {
      col2 = k;
    }
  }

  for (int k = 0; k < 8; k++) {
    if (rows[k] == move_string.charAt(1)) {
      row1 = k;
    }
    if (rows[k] == move_string.charAt(3)) {
      row2 = k;
    }
  }
  led_data_array[row1] |= 1UL << col1;
  led_data_array[row2] |= 1UL << col2;

}


/* ---------------------------------------
 *  Function that displays booting animation.
 *  Writes to specific shift registers and flips states periodically by isr.
 *  @params[in] void
 *  @return void
*/  
void displayBootWait(void) {
  byte boot_led_array[8] = {0};

  if (boot_flipstate) {
    boot_led_array[3] = 0x01;
  }

  digitalWrite(LED_OE_N_PIN , 1);
  digitalWrite(LED_LATCH_PIN, 0);
  shiftOut(boot_led_array);
  DEBUG_SERIAL.println();
  digitalWrite(LED_LATCH_PIN, 1);  
  digitalWrite(LED_OE_N_PIN , 0);
  //analogWrite(LED_OE_N_PIN , LED_DIM_VAL);
}


/* ---------------------------------------
 *  Function that displays move.
 *  Writes to specific shift registers and show start and end position of piece movement.
 *  @params[in] string move
 *  @return void
*/  
void displayMove(String last_move) {
  byte led_test_array[8] = {0};

  setDisplayMove(led_test_array, last_move);

  digitalWrite(LED_OE_N_PIN , 1);
  digitalWrite(LED_MR_N_PIN, 0);
  digitalWrite(LED_MR_N_PIN, 1);
  digitalWrite(LED_LATCH_PIN, 0);
  shiftOut(led_test_array);
  digitalWrite(LED_LATCH_PIN, 1);
  digitalWrite(LED_OE_N_PIN , 0);
  //analogWrite(LED_OE_N_PIN , LED_DIM_VAL);
}
