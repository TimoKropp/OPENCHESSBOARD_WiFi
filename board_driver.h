#pragma once

#include <Arduino.h>
#include "OpenChessBoard.h"

void initHW();
void shiftOut(byte led_data_array[]);
void readHall(byte read_hall_array[]);
String getMoveInput();
void clearDisplay();
void displayConnectWait();
void setDisplayMove(byte led_data_array[], String move_string);
void displayBootWait();
void displayMove(String last_move);
