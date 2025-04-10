#pragma once

extern String GetStringBetweenStrings(String input, String firstdel, String enddel);
extern void checkCastling(String move_input);
extern void setStateBooting(void);
extern void setStateConnecting(void);
extern void setStateUpdating(void);
extern void setStatePlaying(void);
extern String urlDecode(const String &encoded);
extern bool isStartingPosition(void);
extern void readSettings(void);
extern void readBoardSelection(void);