/* ---------------------------------------
 *  setup function for wifi module. 
 *  Checks for wifi firmware  update 
 *  and connects wifi module to network.
 *  @params[in] void
 *  @return void
*/
void wifi_setup(void){
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    DEBUG_SERIAL.println("Communication with WiFi module failed!");
    DEBUG_SERIAL.println("Wifi Failed");
    // don't continue
    while (true);
  }
  
  String fv = WiFi.firmwareVersion();
  
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    DEBUG_SERIAL.println("Please upgrade the firmware");
  }

  DEBUG_SERIAL.println("Attempting wifi");
  DEBUG_SERIAL.println("connection ...");
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    DEBUG_SERIAL.print("Attempting to connect to SSID: ");
    DEBUG_SERIAL.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 1 seconds for connection:
    delay(100);
  }
  DEBUG_SERIAL.println("Connected to wifi");

  printWiFiStatus();
  }

/* ---------------------------------------
 *  function to print wifi status.
 *  @params[in] void
 *  @return void
 *  
*/
void printWiFiStatus(void) {
  // print the SSID of the network you're attached to:
  DEBUG_SERIAL.print("SSID: ");
  DEBUG_SERIAL.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  DEBUG_SERIAL.print("IP Address: ");
  DEBUG_SERIAL.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  DEBUG_SERIAL.print("signal strength (RSSI):");
  DEBUG_SERIAL.print(rssi);
  DEBUG_SERIAL.println(" dBm");
}
