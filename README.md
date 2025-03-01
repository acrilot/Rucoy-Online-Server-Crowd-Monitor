# Description
A tool to monitor amount of online players in servers of Rucoy Online.
This application orders Rucoy Online servers by online characters, refreshes ordered list simultaneously with data source. An LED attached to the pin 19 of ESP32 blinks as data is refreshed.

Data source is 'https://www.rucoyonline.com/server_list.json'

# Important
Insert your Wifi SSID and Password before uploading code to ESP32.

# Materials Used
- ESP32-WROOM-32D
- 128x64 0.96" SSD1306 I2C OLED Display
- 1x LED
- 1x 330Ω Resistor

There is also a python script (Crowd Monitor.py) that does the same job. If you don't have python installed on your computer, install it first. Then download the file and double click on it.
