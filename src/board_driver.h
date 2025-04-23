#pragma once

extern void initHW(void);
extern String getMoveInput(void);
extern void clearDisplay(void);
extern void displayMove(String mv);
extern void displayMoveRecect(String move);
extern void displayBootWait(void);
extern void displayConnectWait(void);
extern void displayUpdateWait(void);
extern void displayArray(byte ledBoardState[]);
extern void readHall(byte read_hall_array[]);
extern void displayFrame(byte frame[8]);
extern void rotate180(byte array[8]);
extern void calculateDifference(byte result[], byte a[], byte b[]);

extern void displayNewGame(void);
extern void displayWaitForGame(void);

extern String createFen(void);
extern bool areFensSame(const String& peripheralFen, const String& centralFen);
