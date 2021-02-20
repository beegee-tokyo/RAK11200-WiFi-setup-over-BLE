# RAK11200-WiFi-setup-over-BLE
|   |   |   |
| :-: | :-: | :-: |
| <img src="./assets/rakstar.jpg" width="150"> | <img src="./assets/Icon.png" width="150"> | <img src="./assets/RAK-Whirls.png" width="150"> |
# WORK IN PROGRESS => NO GUARANTEE THAT IT WORKS ON YOUR PHONE
Setup your ESP32 WiFi credentials over BLE from an Android phone or tablet.
Sometimes you do not want to have your WiFi credentials in the source code, specially if it is open source and maybe accessible as a repository on Github or Bitbucket.

There are already solution like [WiFiManager-ESP32](https://github.com/zhouhan0126/WIFIMANAGER-ESP32) that give you the possibility to setup the WiFi credentials over a captive portal.    
But I wanted to test the possibility to setup the ESP32's WiFi over Bluetooth Low Energy.    
This repository covers the source code for the ESP32. The source code for the Android application are in the [ESP32_WiFi_BLE_Android](https://bitbucket.org/beegee1962/esp32_wifi_ble_android) repository.    

## Older version of ESP32 WiFi setip over BLE
Detailed informations about this project are on my [website](https://desire.giesecke.tk/index.php/2018/04/06/esp32-wifi-setup-over-ble/) (OUTDATED)     

## What's new
By using [h2zero's](https://github.com/h2zero) [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) library the memory usage could be reduced so that the application is now better usable.  
The application to setup the WiFi credentials was as well replaced and has now more functions. Install [My nRF52 Toolbox](https://play.google.com/store/apps/details?id=tk.giesecke.my_nrf52_tb) to get the latest version.

## Development platform
### PlatformIO
See example code in folder ESP32-WiFi-Setup-Over-BLE-Pio    
### Arduino IDE
See example code in folder ESP32-WiFi-Setup-Over-BLE    

## Used hardware
- [WisBlock RAK11200](https://docs.rakwireless.com/Product-Categories/WisBlock/#wisblock-core)		
- Any Android phone or tablet that is capable of BLE.		

## SW practices used
- Use of BLE for sending and receiving data

## Library dependencies		
### PlatformIO lib_deps    
bblanchon/ArduinoJson    
h2zero/NimBLE-Arduino    

### ArduinoIDE
http://librarymanager/All#ArduinoJSON    
http://librarymanager/All#NimBLE-Arduino    