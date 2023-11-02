add_executable(magnetometer magnetometer.c)

target_link_libraries(magnetometer pico_stdlib hardware_i2c hardware_pwm)

pico_enable_stdio_usb(magnetometer 1) # Enable USB serial
pico_enable_stdio_uart(magnetometer 0) # Disable uart

# create map/bin/hex file etc.
pico_add_extra_outputs(magnetometer)

# add url via pico_set_program_url
example_auto_set_url(magnetometer)