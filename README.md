This project provides a simple and efficient WiFi configuration system for ESP-based products. It allows developers to quickly set up and store WiFi credentials via a web interface without hardcoding SSID and password.

Features:
  Web-Based WiFi Setup: Easily configure WiFi credentials from a browser.
  SPIFFS Storage: Saves WiFi credentials for automatic reconnection after reboot.
  AP + STA Mode: Runs in both Access Point (AP) and Station (STA) mode for flexible connectivity.
  Minimal & Lightweight: Designed for rapid development and testing of ESP8266-based products.
How It Works:
  1.The ESP starts in AP mode (CustomWiFiSetup).
  2.Connect to the AP and open 192.168.4.1 in a browser.
  3.Enter and save WiFi credentials via the web interface.
  4.The device attempts to connect to the specified network
