#pragma once
#include "openchessboard.h"

extern void postMove(WiFiClientSecure  &client);
extern void getStream(WiFiClientSecure  &client);
extern void disableClient(WiFiClientSecure  &client);
extern void getGameID(WiFiClientSecure  &client);
extern void postNewGame(WiFiClientSecure  &client, String board_gameMode);
extern char* catchResponseFromClient(WiFiClientSecure &client);
extern String parseValueFromResponse(const char* response, const char* key);