idf_component_register(
    SRCS 
        "main.cpp"
        "zmlib.c"
        "driver_mpu9250_basic.c"
        "src/driver_mpu9250.c"
        "interface/driver_mpu9250_interface.cpp"
        "interface/zm_wifi.c"
        "interface/SPIbus.cpp"
        
    INCLUDE_DIRS
        "."
        "src"
        "interface"
)
