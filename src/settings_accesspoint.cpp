#include "openchessboard.h"
#include "html_content.h"
#include <Logger.h>

// Create a web server on port 80 (default HTTP port)

// Define Wi-Fi AP (Access Point) credentials
const char* ssidAP = "OPENCHESSBOARD"; // SSID for the ESP32 access point
const char* passwordAP = "";
const char* domainName = "OPENCHESSBOARD";
// Create a web server on port 80 (default HTTP port)
WiFiServer APserver(80);

Preferences preferences;


// Function to extract form data from the POST request
String getFormData(String request, String key) {
  int startIndex = request.indexOf(key + "=");
  if (startIndex == -1) return "";
  
  startIndex += key.length() + 1;  // Skip the key part
  int endIndex = request.indexOf("&", startIndex);
  if (endIndex == -1) endIndex = request.length();
  
  return request.substring(startIndex, endIndex);
}

void AP_setup(WiFiServer &APserver) {
  // Start ESP32 in Access Point mode
  if (WiFi.softAP(ssidAP, passwordAP)) {
    LOG_INFO << "Access Point started";
    LOG_INFO << "SSID: " << ssidAP;

    // Display IP address
    IPAddress IP = WiFi.softAPIP();
    LOG_INFO << "AP IP address: " << IP;

    // Start the server
    APserver.begin();
    LOG_INFO << "AP Server started";

    // Initialize mDNS
    if (MDNS.begin(domainName)) {
      LOG_INFO << "mDNS responder started. Access your device at http://" << domainName << ".local";
    } else {
      LOG_INFO << "Error setting up mDNS responder!";
    }
  } else {
    LOG_INFO << "Failed to start Access Point";
  }
}

bool handleAPClientRequest(WiFiClient &client) {
  String currentLine = "";
  bool is_submitted = false;
  String ssid = "";
  String password = "";
  String token = "";
  String gameMode = "";
  String startupType = "";

  // Wait for data from the client
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      Serial.write(c);  // Optional, to debug the received request

      // Read the HTTP request line
      if (c == '\n') {
        // Check for the end of the request header
        if (currentLine.length() == 0) {
          // Send HTTP headers
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("");
          // Serve the HTML content from the string
          LOG_INFO << "HTML CONTENT";
          LOG_INFO << htmlContent;
          client.print(htmlContent);  // Use the simple array for HTML content

          break;
        } else {
          // Reset current line after reading the header line
          currentLine = "";
        }
      } else if (c != '\r') {
        // Add to current line
        currentLine += c;
      }

      // If form is submitted
      if (currentLine.endsWith("POST /submit")) {
        String requestBody = client.readString();
        // Parse the form data
        ssid = getFormData(requestBody, "ssid");
        password = getFormData(requestBody, "password");
        token = getFormData(requestBody, "token");
        gameMode = getFormData(requestBody, "gameMode");
        startupType = getFormData(requestBody, "startupType");

        // Save to flash
        preferences.begin("settings", false); // read/write
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.putString("token", token);
        preferences.putString("gameMode", gameMode);
        preferences.putString("startupType", startupType);
        preferences.end();

        // Send a confirmation page
        String confirmation = "<html><head><style>\n";
        confirmation += "body { font-family: Arial, sans-serif; background-color: #5c5d5e; color: #ec8703; text-align: center; padding: 20px; }\n";
        confirmation += "button { background-color: #ec8703; color: white; border: none; padding: 15px; font-size: 16px; border-radius: 5px; cursor: not-allowed; }\n";
        confirmation += "</style></head><body>";
        confirmation += "<h2>Settings Submitted!</h2>";
        confirmation += "<p>SSID: " + ssid + "</p>";
        confirmation += "<p>Password: " + password + "</p>";
        confirmation += "<p>Token: " + token + "</p>";
        confirmation += "<p>Game Mode: " + urlDecode(gameMode) + "</p>";
        confirmation += "<p>Startup Type: " + startupType + "</p>";
        confirmation += "<p>Restarting the device... Please wait.</p>";
        confirmation += "<button disabled>Restarting...</button>";
        confirmation += "</body></html>";

        client.print(confirmation);
        is_submitted = true;
        break;
      }
    }
  }

  // Close the connection
  client.stop();
  return is_submitted;
}


void run_APsettings(void){
  AP_setup(APserver);
  bool settings_updated = false;
  while(!settings_updated){
    WiFiClient client = APserver.available();

    if (client) {
      settings_updated = handleAPClientRequest(client);
      LOG_INFO << "Client handling...";
    }
  }
  delay(2000);
  LOG_INFO << "Restarting";
  ESP.restart(); 

}