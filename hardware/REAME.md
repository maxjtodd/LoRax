# HARDWARE BRANCH 
### Readme 

## Instroduction

This is the readme file for the hardware branch of the LoRax Project. 

The (C++) code detailed in this document is to be flashed and run on ESP32s, in conjunction with the Swift code running on the associated iOS device.

*NOTE* This README only details the documentation of the hardware aspect of LoRax. For documentation of the iOS aspects, see the respective folder. 

## System Requirements

As stated, this code runs on an ESP32. For testing, this project utilizes the [Heltec Automation Wifi LoRa 32(V3)](https://heltec.org/project/wifi-lora-32-v3/). 

This device comes with an antenna used for broadcasting and receiving packets over the LoRa frequency band, as well as functionality for running a Bluetooth Low Energy server. 

Both of these functionalities are core requirements for LoRa. 

## Installation Instructions

Installation instructions for the hardware code of LoRa are as follows:

 1. Download [Arduino IDE](https://www.arduino.cc/en/software).

 2. Launch Arduino IDE, navigate to packet manager.

 3. Install package "Heltec ESP32 Dev-Boards" - version 1.1.5.

    ![Library Manager](./res/library_manager.png)

 4. Plug in ESP32 device to computer running Arduino IDE. (via USB-C)

 5. Open cloned sketch in hardware branch. (multithread_both_w_import)

 6. Select correct port corresponding to ESP32 device at the top of the window. 

    ![Board Selection](./res/board_select.png)

7. Compile, flash code to ESP32 device. 

## Architecture Overview

The multi-core nature of the ESP32 devices is crucial to our messages implmentation. 

Heltec ESP32 contains two CPU cores. In our implementation, each core is responsible for a different aspect of our messaging.

![ESP32 Processor Diagram](./res/esp32_diagram.png)

With Core 1 running the Bluetooth code, and Core 2 running the LoRa code. The the following diagram for more detail on architecture.

![Code Architecture Diagram](./res/architecture_diagram.png)

- Our primary thread begins with the Arduino Skectch. From here, two concurrent threads are initialized on each respective core. 

- **Bluetooth Core** - This core is responsible for communication between the ESP32, and the iOS app. Functionalities include:

    1. Transferring messages received by LoRa Core to iOS App
    2. Transferring messages received by iOS App to LoRa Core, to be broadcast over Lora. 

- **LoRa Core** - This core is responsible for communcation betweent the host ESP32, and other ESP32 devices reachable by the host. Functionalities include: 

    1. Sending messages from Bluetooth Core over LoRa (to other ESP32s)
    2. Tranferring recevied messages (over LoRa) to Bluetooth Core.
    3. Handling contact pings, transfer new contact pings to Bluetooth Core to populate reachable contacts.
    4. Storing messages sent over LoRa for re-messaging (until acknowledgement is recevied)
    5. Acknowledging received messages.

- **Core Communication** - Cores communication with each other by adding packets to two respective shared list.

    LoRa Core watches one list for new messages from Bluetooth Core, while Bluetooth Core does the same with the other list. MUTEX locks are included with actions performed on each list to ensure race condictions are not ecountered. 


