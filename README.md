# OpenChessBoard
# Requirements
This project runs on Arduino Nano 33 IoT with the OpenChessBoard hardware. You can order the PCBA (inlcuding Arduino Nano 33 IoT) from [OpenChessBoard.com](http://openchessboard.com/).

## Setup
### 1. Download the Arduino IDE
Get the latest version [here](https://www.arduino.cc/en/software).
### 2. Import .ino files from this repository
Download this project and import files with the Arduino IDE.
### 3. Update certificatse for SSL connection of the WiFi module
Update the WiFi module to the latest WiFi firmware and add the lichess.org:443 (instead of google.com:443) root certificate as shown in this [example](https://support.arduino.cc/hc/en-us/articles/360016119219-How-to-add-certificates-to-Wifi-Nina-Wifi-101-Modules-)

# Acknowledgements
Without Lichess.org and their well documented [open-source API](https://lichess.org/api) this project would not be possible.
Special thanks to the [Lichess-Link project](https://github.com/Kzra/Lichess-Link): a great introduction on how to implement a lichess-client on Arduino 

