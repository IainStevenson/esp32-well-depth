# Requirements as Understood

## WiFi Connection:

Connect to a preferred SSID if provided.
If the preferred SSID is unavailable or has weak RSSI, scan and connect to the strongest available network.
Print the MAC address upon successful connection.

### Endpoints:

/:
Returns device information including IP address, MAC address, UTC time, number of data entries, and memory used.

/data:
Returns the collected sensor data in JSON format.
Clears the stored data after returning it.

/data&reset:
Returns the collected sensor data in JSON format.
Clears the stored data after returning it.
Resets the ESP after sending the response.

/monitor:
Returns the collected sensor data in a table format for monitoring.

## Sensor Data Collection:

Measure distance using the ultrasonic sensor at regular intervals (every 10 seconds).
Calculate water depth as MAX_DEPTH_CM - distance.
Store the sensor data with the current timestamp and depth.
Manage sensor data in a FIFO manner to prevent memory overflow.

## Timestamp:

Store the timestamp as an unsigned long.
Format the timestamp as "YYYY-MM-DD HH:MM" when outputting it.

## Error Handling:

Retry WiFi connection every 3 minutes if disconnected.
Handle preferred SSID connection attempts with a timeout of 10 seconds.
Handle data storage and clearing appropriately.

## Code Comments:

Provide liberal comments to explain the code, aiming to educate new developers on the implementation details.