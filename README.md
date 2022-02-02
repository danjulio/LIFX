## LIFX

This is a library to add LIFX control to an ESP8266 or ESP32 project. Use it to discover LIFX lights on your LAN and control their color, power and brightness by specific device, label or group name.

It uses the LIFX LAN Protocol (UDP) rather than the HTTP API.

Originally from Peter Hum's [Github](https://github.com/peter-hum/LIFX) repository.  Thanks Peter!

Additional features in this version:

1. Support for ESP32.
2. Support for color capable lights (SetDeviceColor, SetColorByGroup, SetColorByLabel).
3. Query Product and store in device table during discovery.
4. New LifxProducts library to allow lookup of product name and other characteristics based on Product.
5. New method GetIndexedDevice to get access to internal Devices array.
6. New methods StartDeviceLightUpdate and DeviceLightUpdateDone to get current values from a device.
7. PrintDevices method prints Product and Product Name