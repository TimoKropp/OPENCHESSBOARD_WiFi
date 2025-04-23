#include "openchessboard.h"

byte swapBits(byte b) {
    byte swapped = 0;
    for (int i = 0; i < 8; i++) {
        swapped |= ((b >> i) & 0x01) << (7 - i);
    }
    return swapped;
}

bool isFrameEmpty(byte frame[]) {
  for (int i = 0; i < 8; i++) {
    if (frame[i] != 0) {
      return false;  
    }
  }
  return true; 
}

bool isFrameFull(byte frame[]) {
  for (int i = 0; i < 8; i++) {
    if (frame[i] != 0xFF) {
      return false; 
    }
  }
  return true;  
}

void highlightConnection(byte frame[], int startRow, int startCol, int endRow, int endCol) {
    int rowStep = (endRow > startRow) ? 1 : (endRow < startRow) ? -1 : 0;
    int colStep = (endCol > startCol) ? 1 : (endCol < startCol) ? -1 : 0;

    int row = startRow;
    int col = startCol;

    while (row != endRow || col != endCol) {
        frame[row] |= (1 << col);  

        if (row != endRow) row += rowStep;
        if (col != endCol) col += colStep;
    }

    frame[endRow] |= (1 << endCol);
}
void highlightAttackedSquares(byte hallBoardState[], byte frame[]) {

  for (int i = 0; i < 8; i++) {
    frame[i] = 0x00; 
  }

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      if (hallBoardState[row] & (1 << col)) {  

        frame[row] = 0xFF; 

        for (int r = 0; r < 8; r++) {
          frame[r] |= (1 << col);  
        }

        for (int i = 1; i < 8; i++) {
          if (row + i < 8 && col + i < 8) {
            frame[row + i] |= (1 << (col + i)); 
          }
          if (row - i >= 0 && col - i >= 0) {
            frame[row - i] |= (1 << (col - i));  
          }
        }

        for (int i = 1; i < 8; i++) {
          if (row + i < 8 && col - i >= 0) {
            frame[row + i] |= (1 << (col - i)); 
          }
          if (row - i >= 0 && col + i < 8) {
            frame[row - i] |= (1 << (col + i)); 
          }
        }
      }
    }
  }
  for (int i = 0; i < 4; i++) {

    byte temp = frame[i];
    frame[i] = swapBits(frame[7 - i]);
    frame[7 - i] = swapBits(temp);
  }
}

void highlightAttackedPieces(byte hallBoardState[], byte frame[]) {

    for (int i = 0; i < 8; i++) {
        frame[i] = 0x00;  
    }

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (hallBoardState[row] & (1 << col)) {  

                for (int c = 0; c < 8; c++) {
                    if (c != col && (hallBoardState[row] & (1 << c))) { 
                        highlightConnection(frame, row, col, row, c);
                    }
                }

                for (int r = 0; r < 8; r++) {
                    if (r != row && (hallBoardState[r] & (1 << col))) {  
                        highlightConnection(frame, row, col, r, col);
                    }
                }

                for (int i = 1; i < 8; i++) {

                    if (row + i < 8 && col + i < 8 && (hallBoardState[row + i] & (1 << (col + i)))) {
                        highlightConnection(frame, row, col, row + i, col + i);
                    }
                    if (row - i >= 0 && col - i >= 0 && (hallBoardState[row - i] & (1 << (col - i)))) {
                        highlightConnection(frame, row, col, row - i, col - i);
                    }

 
                    if (row + i < 8 && col - i >= 0 && (hallBoardState[row + i] & (1 << (col - i)))) {
                        highlightConnection(frame, row, col, row + i, col - i);
                    }
                    if (row - i >= 0 && col + i < 8 && (hallBoardState[row - i] & (1 << (col + i)))) {
                        highlightConnection(frame, row, col, row - i, col + i);
                    }
                }
            }
        }
    }

    for (int i = 0; i < 4; i++) {

        byte temp = frame[i];
        frame[i] = swapBits(frame[7 - i]);
        frame[7 - i] = swapBits(temp);
    }
}


void flickeringAnimation(byte frame[]) {

    const int flickerCount = 10; 
    const int delayTime = 1; 

    for (int i = 0; i < flickerCount; i++) {

        for (int k = 0; k < 8; k++) {
            frame[k] = random(0x00, 0xFF);
        }

        displayFrame(frame); 
        delay(delayTime);    
    }

    for (int k = 0; k < 8; k++) {
        frame[k] = 0x00;
    }

    displayFrame(frame); 
}

int countQueens(byte hallBoardState[]) {
    int count = 0;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (hallBoardState[row] & (1 << col)) {
                count++;
            }
        }
    }
    return count;
}

void run_queen_puzzle(void) {
    dimLEDs = true;
    while (true) {
        static bool flickeringShown = false; 
        byte hallBoardState[8];  
        byte frame[8]; 

        readHall(hallBoardState);

        highlightAttackedPieces(hallBoardState, frame);

        if (isFrameEmpty(frame)) {
            highlightAttackedSquares(hallBoardState, frame);
        }

        int queenCount = countQueens(hallBoardState);

        if (isFrameFull(frame) && queenCount == 8) {
            if (!flickeringShown) { 
                flickeringAnimation(frame);
                flickeringShown = true;
            }
            delay(5000);
            ESP.restart();  
        } else {
            flickeringShown = false; 
        }

        displayFrame(frame);
    }
}