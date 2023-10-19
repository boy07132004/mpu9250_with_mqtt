## Introduction
This project utilizes the Xiao ESP32-S3 to collect acceleration data from an MPU9250 sensor. It leverages MQTT for both command control and data transmission. 

## Hardware Requirements
- Xiao ESP32-S3
- MPU9250 Accelerometer
- MQTT Broker

## Software Requirements
- esp-idf (v5.2 in my env)
- Python3
- paho-mqtt (v1.6.1 in my env)

## Installation

1. Install esp-idf & paho-mqtt for python
2. Clone this repository to your local machine.
3. Setup the mqtt configuration in menuconfig and flash the project into Xiao ESP32-S3
    ![](/src/esp-idf.png)
4. Execute python application to collect acceleration data
    ![](/src/python_save_data.png)
5. You will see the file in "save" folder.

## LIBRARY REF
- https://github.com/hepingood/mpu9250
- https://github.com/natanaeljr/esp32-SPIbus