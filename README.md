# LED Controller

This is a bluetooth server written for an ESP32 to allow for control over a strand of LEDs. Via bluetooth, the user can control the animation, the color, the brightness, and the animation speed. 

Currently, it supports 4 different animations:
- Fill Solid
- Color Fade
- Color Wipe
- Meteor (fading color wipe)

### WiFi 
In order to connect to wifi, you must upload a txt file to SPIFFS where the first line is the ssid, and the second line is the password. This also enables OTA updates.