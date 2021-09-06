#Zigbee/Mqtt smart home server

##Overview

It is a simple smart home server using zigbee wireless for devices and mqtt protocol for remote control. A "functional unit" is intended for combining several logically related devices. They are of the following types: onoff, alarm, climate, light, thermostate, level_control, illuminance_level. Using mqtt commands can pairing a zigbee device into a specific functional unit. When connected to a network of a supported device, it will be automatically connected to the required role in the functional unit(switch, temperature sensor, etc).

##Mqtt topics

### Main topic
root_topic/zhub supported commands:
- "devices" - list of all zigbee devices is published in the topic /zhub/devices
- "units" -  list of all functional units is published in the topic /zhub/units"
- "public" - current states and settings of all functional units are published

###Device topcs
root_topic/unit_type/unit_name/subtopic

Subtopic expresses a specific function of "functional unit" (power,temperature, etc) for example in root_topic/climate/badroom_climate/humidity published current humidity value. Also, each "functional unit" has a /control topic to control it. Supported commands: clear - deletes all devices on the unit (remain in the zigbee network), permitjoin - allows pairing of zigbee devices for 60 seconds, options - publishes all settings
unit to subtopic /options.

Most topics support the command "?" to publish the current state.

##Using:
- c++17
- Boost (https://www.boost.org, 1.67.0 or later)
- yaml-cpp (https://github.com/jbeder/yaml-cpp, 0.7.0 or later)
- mqtt_cpp (https://github.com/redboltz/mqtt_cpp, 11.1.0 or later)

##Supported zigbee devices:

- Xiaomi WXKG01LM
- Ikea LED1623G12
- Sonoff BASICZBR3
- Xiaomi GZCGQ01LM
- Xiaomi WXKG03LM
- Xiaomi MCCGQ01LM
- Xiaomi WSDCGQ01LM
- Xiaomi QBKG11LM

##Supported zigbee adapters

- Texas Instruments cc2530, 2521 ... (ZNP)

##Settings

YAML file (configuration.yaml) is used to store server settings. For the initial configuration of the server, you must specify the parameters of serial port, mqtt broker and add the required number of empty functional units to the units section. Options and devices fields will be added automatically during server operation and store its current state.

##How to Build

Using CMake to cross-platform building. Please edit my [`CMakeLists.txt`](./CMakeLists.txt) file for your configuration.

##License

Zhub is licensed under the Boost Software License, Version 1.0. See the [`LICENSE_1_0.txt`](./LICENSE_1_0.txt) file for details.