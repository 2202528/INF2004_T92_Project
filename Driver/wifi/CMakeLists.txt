# Set the source file(s) for the wifi executable
add_executable(wifi wifi.c)

# Set compiler definitions for the wifi target
target_compile_definitions(wifi PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"       # Define WIFI_SSID as a string (may be provided during build)
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\" # Define WIFI_PASSWORD as a string (may be provided during build)
        NO_SYS=0            # Define NO_SYS as 0 (generally configured in lwipopts.h)
        LWIP_SOCKET=1       # Define LWIP_SOCKET as 1 (generally configured in lwipopts.h)
        PING_USE_SOCKETS=1  # Define PING_USE_SOCKETS as 1 (generally configured in lwipopts.h)
        )

# Set include directories for the wifi target
target_include_directories(wifi PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}            # Include the current directory
        ${CMAKE_CURRENT_LIST_DIR}/../..      # Include parent directory
        ${PICO_LWIP_CONTRIB_PATH}/apps/wifi  # Include lwIP WiFi app directory
        )

# Link libraries for the wifi target
target_link_libraries(wifi
        hardware_adc
        pico_cyw43_arch_lwip_sys_freertos
        pico_stdlib
        pico_lwip_iperf
        FreeRTOS-Kernel-Heap4 # Link the FreeRTOS kernel and dynamic heap
        )

# Add extra outputs for the wifi target
pico_add_extra_outputs(wifi)

# Enable USB serial output for the wifi target
pico_enable_stdio_usb(wifi 1)
