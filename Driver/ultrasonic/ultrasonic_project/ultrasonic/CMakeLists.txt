add_executable(ultrasonic
        ultrasonic.c
        )


add_library(pico_ultrasonic ultrasonic.h ultrasonic.c)

target_link_libraries(pico_ultrasonic pico_stdlib hardware_gpio hardware_timer)

#target_include_directories(pico_ultrasonic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


add_library(pico_ultrasonic_object ultrasonic.h ultrasonic.c ultrasonicObject.cpp ultrasonicObject.h)

target_link_libraries(pico_ultrasonic_object pico_stdlib hardware_gpio hardware_timer)

#target_include_directories(pico_ultrasonic_object PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

#pull in common dependencies
target_link_libraries(ultrasonic pico_stdlib)

#create map/bin/hex file etc
pico_add_extra_outputs(ultrasonic)

# add url via pico_set_program_url
example_auto_set_url(ultrasonic)

pico_enable_stdio_usb(ultrasonic 1)