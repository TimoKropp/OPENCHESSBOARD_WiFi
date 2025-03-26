
#pragma once
const char htmlContent[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>OPENCHESSBOARD CONFIGURATION</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #5c5d5e;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
    }

    .container {
      background-color: #353434;
      border-radius: 8px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
      padding: 30px;
      width: 100%;
      max-width: 500px;
    }

    h2 {
      text-align: center;
      color: #ec8703;
      font-size: 24px;
      margin-bottom: 20px;
    }

    label {
      font-size: 16px;
      color: #ec8703;
      margin-bottom: 8px;
      display: block;
    }

    input[type="text"], input[type="password"], select {
      width: 100%;
      padding: 10px;
      margin: 10px 0;
      border: 1px solid #ccc;
      border-radius: 5px;
      box-sizing: border-box;
      font-size: 16px;
    }

    input[type="submit"] {
      background-color: #ec8703;
      color: white;
      border: none;
      padding: 15px;
      font-size: 16px;
      width: 100%;
      border-radius: 5px;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }

    input[type="submit"]:hover {
      background-color: #ebca13;
    }

    select {
      font-size: 16px;
    }

    .form-group {
      margin-bottom: 15px;
    }

    .form-group:last-child {
      margin-bottom: 0;
    }

    .note {
      font-size: 14px;
      color: #ec8703;
      text-align: center;
      margin-top: 20px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>OPENCHESSBOARD CONFIGURATION</h2>
    <form action="/submit" method="POST">
      <div class="form-group">
        <label for="ssid">WiFi SSID:</label>
        <input type="text" name="ssid" id="ssid" value="" placeholder="Enter Your WiFi SSID" required>
      </div>

      <div class="form-group">
        <label for="password">WiFi Password:</label>
        <input type="password" name="password" id="password" value="" placeholder="Enter Your WiFi Password" required>
      </div>

      <div class="form-group">
        <label for="token">Lichess Token (get it <a href="https://lichess.org/account/oauth/token" target="_blank" style="color: #eaeeea;">here</a>):</label>
        <input type="text" name="token" id="token" value="" placeholder="Enter Your Lichess Token" required>
    </div>

      <div class="form-group">
        <label for="gameMode">Time Control (WiFi - auto seek):</label>
        <select name="gameMode" id="gameMode">
          <option value="None">None</option>
          <option value="5+3">5+3</option>
          <option value="10+5">10+5</option>
          <option value="15+10">15+10</option>
          <option value="30+20">30+20</option>
          <option value="AI level 1">AI level 1</option>
          <option value="AI level 2">AI level 2</option>
          <option value="AI level 3">AI level 3</option>
          <option value="AI level 4">AI level 4</option>
          <option value="AI level 5">AI level 5</option>
          <option value="AI level 6">AI level 6</option>
          <option value="AI level 7">AI level 7</option>
          <option value="AI level 8">AI level 8</option>
        </select>
      </div>

      <div class="form-group">
        <label for="startupType">Default Startup Type:</label>
        <select name="startupType" id="startupType">
          <option value="WiFi">WiFi</option>
          <option value="BLE">BLE</option>
        </select>
      </div>

      <input type="submit" value="Submit">
    </form>
    <div class="note">
      <p>Make sure to enter the correct WiFi credentials and settings.</p>
    </div>
  </div>
</body>
</html>

)rawliteral";
