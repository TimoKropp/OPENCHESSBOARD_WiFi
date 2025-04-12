#pragma once
#include <Arduino.h>
#include <FreeRTOS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "settings.h"
#include "utility.h"
#include "timer.h"
#include "wifi_client.h"
#include "lichess_client.h"
#include "board_driver.h"
#include "settings_accesspoint.h"
#include <Preferences.h>
#include "ble_app.h"
#include "wifi_app.h"
#include "puzzle_app.h"
#include <Update.h>
#include <esp_ota_ops.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>

// Debug Settings
//#define MANUAL_MOVE_INPUT
//#define PLUG_AT_TOP // not fully supported yet
#define DEBUG true  
#define DEBUG_SERIAL if(DEBUG)Serial

// WiFi variables
extern int status;
extern char server[];  

extern WiFiClientSecure StreamClient;
extern WiFiClientSecure PostClient;

extern hw_timer_t *timer;
extern volatile bool timerFlag;

//lichess variables
extern String username;
extern String currentGameID;
extern bool myturn;
extern String myLastMove;
extern String oppLastMove;
extern String latestMove;
extern String myMove;
extern String moves; 
extern bool is_castling_allowed;

// LED and state variables
extern bool update_flipstate;
extern bool is_booting;
extern bool is_connecting;
extern bool is_updating;
extern bool is_game_running;
extern bool is_seeking;
extern bool dimLEDs;

// Settings object
extern Preferences preferences;

// wifi  and board variablesht
extern String wifi_ssid;
extern String wifi_password;
extern String lichess_api_token;
extern String board_gameMode;
extern String board_startupType;