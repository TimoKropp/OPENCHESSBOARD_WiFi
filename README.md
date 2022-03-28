# What is the OpenChessBoard?
The OpenChessBoard is an open-source smart chess board to play online chess on a physical chess board. 
Check out how it works in this video.<br/>
[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/PFouUxKJOSc/0.jpg)](https://www.youtube.com/watch?v=PFouUxKJOSc)
# OpenChessBoard v1.0.0
Currently the board is linked to your account on Lichess.org and directly connects to any ongoing game. You can use the Lichess browser application or your smartphone app to start a game from your account. When the board finds a new ongoing game, the board serves as input device and directly sends the move inputs to the Lichess server over a secure WiFi connection (SSL).

# Requirements
This project runs on Arduino Nano 33 IoT with the OpenChessBoard hardware. You can order the PCBA (inlcuding Arduino Nano 33 IoT) from [OpenChessBoard.com](http://openchessboard.com/).

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
### Blink patterns after power up
#### All LEDs light up simultainiously (Hardware power-up)
####  a4 blinking (connectiong to wifi)
####  Center squares blinking (searching for ongoing game)
#### All LEDs off (it is your move and it is the first move of the game)
#### Two squares light up  (it is your move but you need to input the last move played)

# Acknowledgements
Without Lichess.org and their well documented [open-source API](https://lichess.org/api) this project would not be possible.
Special thanks to the [Lichess-Link project](https://github.com/Kzra/Lichess-Link): A great introduction on how to implement a lichess-client on Arduino.

