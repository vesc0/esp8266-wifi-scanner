# ESP8266 Wi-Fi Signal Scanner

A **live Wi-Fi network scanner** using the ESP8266 that hosts a web-based dashboard showing nearby Wi-Fi networks and their signal strengths in real-time. The dashboard uses **Server-Sent Events (SSE)** for live updates, visualized with color-coded bars.

## Contents
- [Features](#features)
- [Hardware Required](#hardware-required)
- [Software Required](#software-required)
- [Installation](#installation)
- [Usage](#usage)
- [How it Works](#how-it-works)

## Features

- Acts as a Wi-Fi Access Point (AP) for direct connection.
- Scans nearby Wi-Fi networks periodically (every 5 seconds).
- Displays SSID and RSSI (signal strength) on a responsive web dashboard.
- Signal strength visualization with color-coded bars:
  - **Green:** Strong signal (≥ -50 dBm)  
  - **Yellow:** Moderate signal (-70 to -50 dBm)  
  - **Red:** Weak signal (< -70 dBm)
- Supports up to 10 nearby networks at a time.
- Lightweight and fully asynchronous using `ESPAsyncWebServer`.

## Hardware Required

- ESP8266-based board (e.g., NodeMCU, Wemos D1 Mini)
- USB cable for programming

## Software Required

- Arduino IDE
- ESP8266 board support installed
- Libraries:
  - [ESP8266WiFi](https://github.com/esp8266/Arduino)
  - [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP)
  - [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)

## Installation

1. Install the required libraries via the Arduino Library Manager or manually.
2. Copy the code into a new Arduino sketch.
3. Configure AP credentials if needed:

```cpp
const char* ssid = "ESP8266_Scanner";
const char* password = "12345678";
```

4.	Upload the sketch to your ESP8266 board.
5.	Open the Serial Monitor (115200 baud) to see the AP IP address.
6.	Connect to the ESP8266 Wi-Fi AP using your PC or phone.
7.	Open a browser and navigate to the ESP8266 AP IP (default 192.168.4.1).

## Usage
- Connect your device to the ESP8266 AP.
- Open the dashboard in a browser.
- Nearby Wi-Fi networks will appear as colored bars, updating automatically every 5 seconds.

## How it works
1.	Wi-Fi Scanning:
The ESP8266 scans for available networks every 5 seconds, storing the strongest 10 networks.
2.	Sorting:
Networks are sorted in descending order by RSSI (signal strength).
3.	JSON & SSE:
The ESP8266 sends a JSON object of detected networks to the browser using Server-Sent Events.
4.	Dashboard Rendering:
The web page dynamically updates bars for each network with color and width mapped to RSSI.
