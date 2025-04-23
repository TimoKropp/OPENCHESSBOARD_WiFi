#include "openchessboard.h"


/* ---------------------------------------
 *  setup function for wifi module. 
 *  Checks for wifi firmware  update 
 *  and connects wifi module to network.
 *  @params[in] void
 *  @return void
*/
void wifi_setup(void){

  WiFi.begin(wifi_ssid, wifi_password);

  DEBUG_SERIAL.print("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {
      delay(300);
      DEBUG_SERIAL.print(".");
      displayConnectWait();
  }
  DEBUG_SERIAL.println("");
  
  printWiFiStatus();
  }

/* ---------------------------------------
 *  function to print wifi status.
 *  @params[in] void
 *  @return void
 *  
*/
void printWiFiStatus(void) {
  DEBUG_SERIAL.print("SSID: ");
  DEBUG_SERIAL.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  DEBUG_SERIAL.print("IP Address: ");
  DEBUG_SERIAL.println(ip);

  long rssi = WiFi.RSSI();
  DEBUG_SERIAL.print("signal strength (RSSI):");
  DEBUG_SERIAL.print(rssi);
  DEBUG_SERIAL.println(" dBm");
}

String fetchMetaData(const char* metadata_url) {
  WiFiClientSecure client;
  client.setInsecure();  

  if (!client.connect("raw.githubusercontent.com", 443)) {
    DEBUG_SERIAL.println("Connection to server failed!");
    return "";
  }

  client.print(String("GET ") + metadata_url + " HTTP/1.1\r\n" +
               "Host: raw.githubusercontent.com\r\n" +
               "User-Agent: ESP32\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {  
      DEBUG_SERIAL.println("Timeout waiting for response headers");
      client.stop();
      return "";
    }
  }

  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  String response = client.readString();
  client.stop();
  //delay(1000);

  return response;
}

bool isNewVersionAvailable(String latest_version) {

    preferences.begin("settings", true);
    String current_version = preferences.getString("firmware", "0.0.0");
    preferences.end();
    DEBUG_SERIAL.printf("Current Version: %s, Latest Version: %s\n", current_version.c_str(), latest_version.c_str());

  return (latest_version != current_version); 
}




bool fetchLatestVersion(String& latest_version) {   
    const char* url = "/TimoKropp/OPENCHESSBOARD_WiFi/main_nano_esp32/release/version.json";
    const int maxRetries = 3;  
    const unsigned long retryDelay = 3000;  
    int fetchRetries = 0;

    String metadata;

    while (fetchRetries < maxRetries) {
        metadata = fetchMetaData(url);
        if (!metadata.isEmpty()) {
            break;  
        } else {
            DEBUG_SERIAL.println("Failed to fetch metadata, retrying...");
            fetchRetries++;
            delay(retryDelay);
        }
    }

    if (metadata.isEmpty()) {
        DEBUG_SERIAL.println("Failed to fetch metadata after retries");
        return false;
    }

    int versionIndex = metadata.indexOf("\"latest_version\":");
    if (versionIndex == -1) {
      DEBUG_SERIAL.println("Failed to find latest_version in JSON.");
      return false;
    }

    versionIndex = metadata.indexOf("\"", versionIndex + 17);  
    int endIndex = metadata.indexOf("\"", versionIndex + 1);
    
    if (versionIndex == -1 || endIndex == -1) {
      DEBUG_SERIAL.println("Failed to extract version.");
      return false;
    }

    latest_version = metadata.substring(versionIndex + 1, endIndex);

    return latest_version;
}

bool downloadFirmware(String latest_version) {
    const int maxRetries = 3;  // Maximum number of retries for both fetching and writing
    size_t chunkSize = 5120;
    const int retryDelay = 3000;

    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_SERIAL.println("WiFi not connected");
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure();

    int downloadRetries = 0;
    size_t totalWritten = 0;

    size_t currentByte = 0;
    int contentLength = -1;

    while (downloadRetries < maxRetries) {
        if (client.connect("raw.githubusercontent.com", 443)) {
            DEBUG_SERIAL.println("Connected to server for downloading firmware version: " +  latest_version);

            client.print(String("GET /TimoKropp/OPENCHESSBOARD_WiFi/main_nano_esp32/release/") + "firmware_v" + latest_version +".bin"+ " HTTP/1.1\r\n" +
                         "Host: raw.githubusercontent.com\r\n" +
                         "User-Agent: ESP32\r\n" +
                         "Connection: keep-alive\r\n\r\n");

            unsigned long timeout = millis();
            while (client.available() == 0) {
                if (millis() - timeout > 5000) {
                    DEBUG_SERIAL.println("Timeout waiting for response body");
                    client.stop();
                    break;
                }
            }

            String headers;
            while (client.available()) {
                String line = client.readStringUntil('\n');
                headers += line + "\n";
                if (line.startsWith("Content-Length: ")) {
                    contentLength = line.substring(15).toInt();
                }
                if (line == "\r") {
                    break;
                }
            }

            if (contentLength <= 0) {
                DEBUG_SERIAL.println("Invalid or missing Content-Length header. Aborting.");
                return false;
            } else {
                DEBUG_SERIAL.printf("Total Content Length: %d bytes\n", contentLength);
            }

            if (Update.begin(contentLength)) {
                uint8_t buffer[chunkSize];
                while (client.connected() && totalWritten < contentLength) {
                    String rangeHeader = "Range: bytes=" + String(currentByte) + "-" + String(currentByte + chunkSize - 1);
                    client.print(String("GET /TimoKropp/OPENCHESSBOARD_WiFi/main_nano_esp32/release/") + "firmware_v" + latest_version + " HTTP/1.1\r\n" +
                                 "Host: raw.githubusercontent.com\r\n" +
                                 "User-Agent: ESP32\r\n" +
                                 "Connection: keep-alive\r\n" +
                                 rangeHeader + "\r\n\r\n");

                    unsigned long timeout = millis();
                    while (client.available() == 0) {
                        if (millis() - timeout > 5000) {
                            DEBUG_SERIAL.println("Timeout waiting for response body");
                            client.stop();
                            break;
                        }
                    }

                    size_t len = client.read(buffer, sizeof(buffer));
                    if (len > 0) {
                        size_t written = Update.write(buffer, len);
                        if (written != len) {
                            DEBUG_SERIAL.println("Error: Mismatch in written and read data");
                            break;
                        }
                        totalWritten += written;
                        currentByte += written;
                        DEBUG_SERIAL.printf("Written %d/%d bytes\n", totalWritten, contentLength);

                    } else {
                        DEBUG_SERIAL.println("No data available from client. Retrying...");
                        break;
                    }
                }

                if (Update.end() && Update.isFinished()) {

                    preferences.begin("settings", false);
                    preferences.putString("firmware", latest_version);
                    preferences.end();
                    DEBUG_SERIAL.println("Update finished. Rebooting...");
                    delay(3000);
                    ESP.restart();
                    return true;
                } else {
                    DEBUG_SERIAL.printf("Update failed: %s\n", Update.getError());
                }
            } else {
                DEBUG_SERIAL.println("Failed to begin update.");
            }
        } else {
            DEBUG_SERIAL.println("Failed to connect to server, retrying...");
        }

        downloadRetries++;
        delay(retryDelay);
    }

    return false;
}

void wifi_firmwareUpdate() {
    String latest_version; 
    if (fetchLatestVersion(latest_version)){
      
      if (!isNewVersionAvailable(latest_version)) {
          DEBUG_SERIAL.println("Firmware is up to date!");
          return;
      }
      else{
        DEBUG_SERIAL.println("Download firmware:" + latest_version);
        setStateUpdating();
        update_flipstate = true;
        displayUpdateWait();
        if (!downloadFirmware(latest_version)) {
            DEBUG_SERIAL.println("Firmware update failed");
            return;
        }
      }

    }
    else{
      DEBUG_SERIAL.println("No new Version available");
      return;
    }


}

void validateFirmware() {
    const esp_partition_t* running = esp_ota_get_running_partition();
    DEBUG_SERIAL.printf("Running partition: %s\n", running->label);

    esp_ota_img_states_t ota_state;
    esp_err_t result = esp_ota_get_state_partition(running, &ota_state);

    if (result == ESP_OK) {
        DEBUG_SERIAL.printf("OTA state: %d\n", ota_state);

        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            DEBUG_SERIAL.println("Firmware pending verify...");

            // Do some checks — optional: WiFi, sensors, etc.

            // If all checks pass:
            esp_err_t valid_result = esp_ota_mark_app_valid_cancel_rollback();
            if (valid_result == ESP_OK) {
                DEBUG_SERIAL.println("Firmware marked as valid!");
            } else {
                DEBUG_SERIAL.printf("Failed to mark firmware valid: %s\n", esp_err_to_name(valid_result));
            }

        } else if (ota_state == ESP_OTA_IMG_VALID) {
            DEBUG_SERIAL.println("Firmware already valid.");
        } else if (ota_state == ESP_OTA_IMG_INVALID) {
            DEBUG_SERIAL.println("Firmware marked as invalid.");
        } else if (ota_state == ESP_OTA_IMG_ABORTED) {
            DEBUG_SERIAL.println("Firmware update was aborted.");
        } else {
            DEBUG_SERIAL.printf("Unknown OTA state: %d\n", ota_state);
        }

    } else {
        DEBUG_SERIAL.printf("Failed to get OTA state: %s\n", esp_err_to_name(result));
    }
}
