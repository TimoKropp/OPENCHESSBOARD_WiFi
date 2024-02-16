#pragma once

#include <Arduino.h>
#include "OpenChessBoard.h"
#include "board_driver.h"

String GetStringBetweenStrings(String input, String firstdel, String enddel);
void checkCastling(String move_input);
