/* ---------------------------------------
 *  interupt handler function. Changes LED states for booting and connection sequence.
 *  When game is running, this function periodically checks for the game status from 
 *  the stream of the StreamClient.
 *  @params[in] void
 *  @return void
*/
void TC4_Handler(void)
{
  if (is_booting)
  {
    displayBootWait();
    boot_flipstate = !boot_flipstate;
  }
  
  if (is_connecting)
  {
    displayConnectWait();
    connect_flipstate = !connect_flipstate;
  }
  
  if (is_game_running)
  {
    DEBUG_SERIAL.println("game is running");
    
     char char_response[800] = {0};
     while(StreamClient.available()) {
      char_response[800] = {0};
      StreamClient.readBytesUntil('\n', char_response, sizeof(char_response));
     }
     String game_status_str  = GetStringBetweenStrings((String)char_response, "status", "winner");
     String game_status = game_status_str.substring(3, 10);
     
     DEBUG_SERIAL.println(game_status);
     
     if (game_status != "started" && game_status_str != NULL)
     {
        DEBUG_SERIAL.println(game_status);
        is_game_running = false;
        is_connecting  = true;
     }


  }
  
  TC4->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;             // Clear the OVF interrupt flag

}


/* ---------------------------------------
 *  Function to set up interupt service routine. 
 *  Sets blinking frequency by isr interval.
 *  @params[in] void
 *  @return void
*/
void isr_retup(void) {

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |                 // Enable GCLK0 for TC4 and TC5
                      GCLK_CLKCTRL_GEN_GCLK1 |             // Select GCLK0 at 48MHz
                      GCLK_CLKCTRL_ID_TC4_TC5;             // Feed GCLK0 output to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);                       // Wait for synchronization

  TC4->COUNT16.CC[0].reg = 2000;                          // Set the TC4 CC0 register as the TOP value in match frequency mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization

  NVIC_SetPriority(TC4_IRQn, 0);    // Set the Nested Vector Interrupt Controller (NVIC) priority for TC4 to 0 (highest)
  NVIC_EnableIRQ(TC4_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)

  TC4->COUNT16.INTENSET.reg = TC_INTENSET_OVF;             // Enable TC4 overflow (OVF) interrupts

  TC4->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCSYNC_PRESC |     // Reset timer on the next prescaler clock
                            TC_CTRLA_PRESCALER_DIV8 |      // Set prescaler to 8, 48MHz/8 = 6MHz
                            TC_CTRLA_WAVEGEN_MFRQ |        // Put the timer TC4 into match frequency (MFRQ) mode
                            TC_CTRLA_MODE_COUNT16;         // Set the timer to 16-bit mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization

  TC4->COUNT16.CTRLA.bit.ENABLE = 1;                       // Enable the TC4 timer
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization
}
