# What is the OPENCHESSBOARD?
The OPENCHESSBOARD is an open-source smart chess board to play online chess on a physical chess board. 
Check out how it works in this video.<br/>
[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/PFouUxKJOSc/0.jpg)](https://www.youtube.com/watch?v=PFouUxKJOSc)
# OPENCHESSBOARD v1.0.1
Currently the board is linked to your account on Lichess.org and directly connects to any ongoing game. You can use the Lichess browser application or your smartphone app to start a game from your account. When the board finds a new ongoing game, the board serves as input device and directly sends the move inputs to the Lichess server over a secure WiFi connection (SSL).

# Requirements
This project runs on Arduino Nano 33 IoT with the OPENCHESSBOARD hardware. You can order the PCBA (inlcuding Arduino Nano 33 IoT) from [OPENCHESSBOARD.com](http://openchessboard.com/).

## Setup
### 1. Download the Arduino IDE
Get the latest version [here](https://www.arduino.cc/en/software).
### 2. Import .ino files from this repository
Download this project and import files with the Arduino IDE.
### 3. Update certificate for SSL connection of the WiFi module
Update the WiFi module to the latest WiFi firmware and add the lichess.org:443 (instead of google.com:443) root certificate as shown in this [example](https://support.arduino.cc/hc/en-us/articles/360016119219-How-to-add-certificates-to-Wifi-Nina-Wifi-101-Modules-).
### 4. Link the OpenChessBoard to your Lichess account
Generate an API token [here](https://lichess.org/account/oauth/token).
### 5. Change your login data in OpenChessBoard.ino
Input your personal WiFi login credentials as well as your Lichess token.

## How to play
### 1. Blink patterns after power up
- All LEDs light up simultaneously (Hardware power-up)
-  a4 blinking (connecting to wifi)
- Center squares blinking (searching for ongoing game)
- All LEDs off (it is your move and it is the first move of the game)
- Two squares light up  (it is your move but you need to input the last move played)
### 2. How to move
- 	**Pick and place:** When picking up the piece you want to move, the LED lights up where the piece was originally.
You can keep the piece as long as you want in your hand and when you place it on the end square,
the board registers this square by lighting up the LED. 
- 	**Slide:** While moving a piece via sliding, the board blocks all unintentional inputs until you keep the piece at rest for about 300ms.
### 3. How to capture
The board works by registering the start square of a move first and then waiting for the end square where the piece moves. 
Therefore when capturing a piece you need to pick up the piece you want to move first and only then pick up the piece you want to caputure.
- **One handed:** You could use one hand to pick up both pieces starting with your piece first.
- **Two handed:** You could use two hands to pick up your piece first and use the other hand to take the captured piece.
To ensure proper move input wait until the LED lights up below the captured piece and only then place your piece on the square.
### 4. Castling
Castling registeres as king move which means you need to move your king first and the jump the rook over.
### 5. Take back (wrong move)
A wrong move is indicated by lighting up both squares of the move you just played. 
You need to play the move in reverse as take back and if all LEDs are off again you can make a different move.
### 6. Unexpected issues
- Only one square lights up below a piece you havent moved yet. 
This means the piece was slightly misplaced and the board registers a wrong starting square as move input.
Just lift the piece on that square and wait until the LED turns off. If all LEDs are off again, you can make your move as usual.
- The board keeps requesting take backs.
This means you lost WiFi connection and need to reconnect to WiFi and the ongoing game. Unplug the board  wait 1 second and reconnect the power. 
The board then reconnects again to the game and starts from the last known position.
- If you somehow knockover multiple pieces the most simple way is to unplug the board, reset all pieces and then power it up. 
This process only takes a few seconds and the board reconnects to the game and starts from the last known position.
# Acknowledgements
Without Lichess.org and their well documented [open-source API](https://lichess.org/api) this project would not be possible.
Special thanks to the [Lichess-Link project](https://github.com/Kzra/Lichess-Link): A great introduction on how to implement a lichess-client on Arduino.

