#pragma once

#include <Arduino.h>
#include <WiFiNINA.h>
#include "board_driver.h"
#include "lichess_client.h"
#include "timers.h"
#include "utility.h"
#include "wifi_client.h"

// Debug settings
#define DEBUG true // set to true for debug output, false for no debug output
#define DEBUG_SERIAL if (DEBUG) Serial

// User settings
extern char ssid[];
extern char pass[];
extern char token[];

// WiFi variables
extern int status;
extern char server[];
extern WiFiSSLClient StreamClient;

// Lichess variables
extern const char* username;
extern const char* currentGameID;
extern bool myturn;
extern String lastMove;
extern String myMove;
extern bool is_castling_allowed;

// LED and state variables
extern bool boot_flipstate;
extern bool is_booting;
extern bool isr_first_run;
extern bool connect_flipstate;
extern bool is_connecting;
extern bool is_game_running;
