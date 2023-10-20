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

1. Setup esp-idf in your local machine
2. Pip install paho-mqtt
3. Clone this repository to your local machine.
4. Setup the configuration in menuconfig
    
    ![](/src/mqtt_config.png)

5. Flash the project into Xiao ESP32-S3
    ```bash
    idf.py flash
    ```
6. Execute python application to collect acceleration data

    ![](/src/python_save_data.png)
7. You will see the file in "save" folder.

## LIBRARY REF
- https://github.com/hepingood/mpu9250
- https://github.com/natanaeljr/esp32-SPIbus