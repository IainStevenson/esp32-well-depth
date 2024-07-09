# Project Overview

To produce a Visual Studio Code platformIO solution for an Expressif ESP-WROOM-32 device that satisfies the following requirements;

# Requirements

## Functional Requirements

1. Measure the current depth of water in a domestic well via its 'known max depth' (assumed to be 300 CM) minus its current 'distance' from the ultrasonic sensor.
2. Calcualtes the sensors distance from the water level using its returned time value converted to CM, using the speed of sound in cmd per microsecond deducted from the known max depth value.
3. Stores the calculated depth data along with the current UTC time in a FIFO managed memory array, with access to the array protected by a mutex
4. The in memory array is to be sized to use no more than 25% of the available memory at the end of setup.
5. The NNTP time values are to be stored as unsigned long but converted as String of format YYYY-MM-DD HH:MM:SS on output to any retrieval endpoint
6. the device will automatically connect to the strongest available WIFI ssid endpoint available using the password supplied by the MyWIFICreds.h file if the SSID value is blank
7.  If the ssid value is set then it will only attemtp to connect to that WIFI endpoint. 
8. Pauses and displays a 'Please reset!' message if no WIFI endpoint is found to be available or the defined SSID rejects or is not available.
9. If the network is lost during operation the last used ssid connection is retried indefinately without losing the existing data in accordance with the FIFO.
10. if after 5 minutes the ssid is no longer available then scan for any available endpoint and retry as if none were specified on startup.
11. The ultrasonic data is gathered every 10 seconds.
12. The device can be queried asynchronously using HTTP on these endpoints; /  /reset/ data /monitor
13. No security requirements are needed for the device endpoints.
14. The /monitor endpopint produces an HTML data table of the array data in reverse order
15. The /data endpoint produceds Json data output and clears the array down
16. The /data endpoint with the reset parameter will perform as for /Data then additionally reset the device

## Non functional requirements
# Code files
1. Declare for inclusion in the platformio.ini file which Libraries need to be added to the project as part of the platformio.ini file itself
2. Assume Serial communications is the default and at a BAUD of 115200 
3. Assume the upload speed is left as default


# Technical Specifications
- Programming Language: [c++]
- Framework: [Arduino]
- Libraries: [To be specified]
- Tools: [VS Code/PlatformIO]

# Data Models
## SensorData
- depth: float
- time: unsigned long 


# APIs
## Endpoint /
- Method: GET
- URL: /
- Request Parameters: [none]
- Response: [HTML]

## Endpoint /data
- Method: GET
- URL: /
- Request Parameters: [none]
- Response: [Json]

## Endpoint /data
- Method: GET
- URL: /
- Request Parameters: [reset]
- Response: [Json]

## Endpoint /monitor
- Method: GET
- URL: /monitor
- Request Parameters: [none]
- Response: [HTML]


# During setup
Initialise the sensor pins
Initialise the WIFI client
Scan the Wifi SSId's and choose the best one based on RSSI using the password supplied in MyWFICreds.h if the SSID is left blank in MyWIFICreds.h
If the connection is rejected by the WIFI endpoint then try the next best endpoint regadless of if an ssid was specified
On failure to connect to the network on any SSID, Outut a 'Please reset' message then wait forever
On connection to the WIFI, output the device IP address and MAC address
Setup to use NNTP (as UTC) to synchronise the time and wait for synchronisation to happen and then output the UTC time
Initialise the FIFO 'sensorData' array to use approx 25% of the available free memory


# During Loop
Get the time update
If network connection is lost, and no recent retry is being currently attempted, asynchronously perform a retry of the last, on failure of that retry any that work. If non are working wait 2 minutes and retry but keep recording data.
Scan the ultrasonic sensor to get the water level and convert it to the water depth as described above.
Write a new 'SensorData' instance using the Time now and the depth to the FIFO 'sensorData' array 
Wait just long enough to start again 10 seconds after the the last scan of the sensor


Asynchronously
On Connection from a client 
using the "/" endpoint output to the client the devices IP address, MAC address and the current time as UTC and the number of entries in the FIFO 'SensorData' Array, the meory available and the memory used so far.
using the "/monitor" endpoint output the FIFO SensorData' array in reverse order as an HMTL table with an HTML refresh of 10 seconds
using the "/data" endpoint output to the client the entire FIFO SensorData' array as Json then clear the array.
using the "/data&reset" endpoint process as for teh non reset /Data endpoint, then reset the device.



# Mockups/Examples
None
## Example Input
No input