# ESPHome-Network-Port-Monitor
ESP32 based Network port Monitor built in ESPHome

# Overview
This is a simple network monitor for ESP/Home and Home Assistant.
You can monitor any IP accessible from the ESP device and ports.
The example code in monitor.yaml include a web_server block thats just a nice to have, delete if you do not need it ;-)

## Validated test environment
This code is only tested on ESPHome version 2026.4 on a ESP32-C6 device. You see this in the esp32 block of the yaml code.

# Installation
You need a ESP device having basic configuration and connected to ESPHome (and Home Assistant)
1. Copy the file netcheck.h to your ESPHome folder (same foder as your device yaml file
2. Copy the code in monitor.yaml to your device in ESPHome, please note the file include example of how the basic blocks may look like.
The importen part is that you get the includ in the top block:
esphome:
  includes:
    - netcheck.h

"binary_sensor" have the objects you like  to monitor

"sensor" have extra measurments like ssh latency

"text_sensor" have a last ploblem overview filed

"interval" this is the part where the configuring of the monitoring takes place.
 - "interval: 30s" - How often to monitor the IP
 - "startup_delay: 20s" - Waiting for the ESP to startupo and seperate the monitors to not run at the same time.
 - "lambda"
   - Give auto a uniq id for the monitor like http0
   - Set your monitor inputs; IP, Port, TimeOut
   - set your ID from binary_sensor
   - Set a friendly name for your monitor
  
Like:
```
  - interval: 30s
    startup_delay: 25s
    then:
      - lambda: |-
          auto http0 = netmon::tcp_probe("10.10.10.20", 80, 700);
          id(router_my_iot_router_http).publish_state(http0.ok);
            if (!http0.ok) {
              id(net_last_problem).publish_state(std::string("Router My IoT Router: ") + http0.error);
            }
```
