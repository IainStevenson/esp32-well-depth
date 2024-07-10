# Project Overview

To produce a Visual Studio Code platformIO solution for an Expressif ESP-WROOM-32 device that satisfies the following requirements;

# Requirements

## Functional Requirements

1. Calcualte the sensors distance from the water level using its returned time value converted to CM, using the speed of sound in cmd per microsecond.
2. Calcualte the current depth of water in a domestic well via its 'known max depth' (assumed to be 300 CM) minus its current 'distance' from the ultrasonic sensor.
3. Store the calculated depth data along with the current UTC time in a FIFO managed memory array, with access to the array with concurrency protection using a mutex
4. The data array is to be sized to use no more than 25% of the available memory at the end of setup. See 'additional info'.
5. The NNTP time values are to be stored as unsigned long but converted as String of format YYYY-MM-DD HH:MM:SS on output to any retrieval endpoint
6. The device will automatically connect to either a preferred SSID or if not specified, the strongest available WIFI SSID endpoint available using the password supplied by the MyWIFICreds.h file.
7. If the preferred SSID is not available then revert to scanning available SSID's and use in best order by RSSI.
8. Pauses and displays a 'Please reset!' message if no SSID endpoint is found to be available or the defined SSID rejects, or is not available.
9. If the network is lost during operation the last used SSID connection is retried indefinately without losing the existing data in accordance with the FIFO capacity.
10. If after 3 minutes the SSID is no longer available then scan for any available endpoint and retry as if none were specified on startup.
11. The ultrasonic data is gathered every 10 seconds, kept in teh FIFO array, regardless of netowkr status. But is only started once a network connection is obtained.
12. The device can be queried asynchronously using HTTP on these endpoints; /  /data /data&reset /monitor
13. No security requirements are needed for the device endpoints.
14. The /monitor endpopint produces an HTML data table of the array data in reverse order
15. The /data endpoint produceds Json data output and clears the array down
16. The /data&reset endpoint will perform the same as for /data then additionally reset the device.

## Non functional requirements
# Code files
1. Declare for inclusion in the solution file which Libraries need to be added to the project as part of the platformio.ini file itself
2. Assume Serial communications is the default and at a BAUD of 115200 
3. Assume the upload speed is left as the fastest supported by the platform.
4. Noteote any special setup instructions for Linux regarding permissions and hardware access.
5. Comment the solution as if it were an educational example

# Technical Specifications
- Programming Language: [c++]
- Framework: [Arduino]
- Libraries: [To be specified]
- Tools: [VS Code/PlatformIO plugin].  

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

## Endpoint /data with reset
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
Initialise the ultrasonic sensor pins on D5 (Trigger) and D18 (Echo)
Initialise the WIFI client
Scan the Wifi SSId's and choose the best one based on RSSI using the SSID and password supplied in MyWFICreds.h 
If the SSID is blank then start scanning the SSIDS and try the password in best order by RRSI value.
If the SSSID is not blank then try that first anyway and then resort to scanning for any available SSID and try each in order of best RSSI value.
If no SSID is available, or can connect, then announce 'No SSIDS found', wait 10 seconds and start again.
Assume the password works for all observable WIFI SSID's
On connection to the WIFI, output the device IP address and MAC address
Setup to use NNTP (as UTC) to synchronise the time and wait for synchronisation to happen and then output the UTC time
Initialise the Ethernet Server for asynchornous client calls.
Initialise the FIFO 'sensorData' array to use approx 25% of the available free memory.


# During Loop
If no network connection, and no recent retry is being currently attempted, asynchronously perform the SSID connection strategy described above. Keep recording data.
if the network is available Get the NNTP client time update otherwise guess what teh time is based on elapsed milliseconds from the last known network time.

Scan the ultrasonic sensor to get the water level and convert it to the water depth as described above.
Write a new 'SensorData' instance using the Time now and the depth to the FIFO 'sensorData' array 

Wait just long enough to start again 10 seconds after the the last scan of the sensor


Asynchronously
On Connection from a client 
using the "/" endpoint output to the client the devices IP address, MAC address and the current time as UTC and the number of entries in the FIFO 'SensorData' Array, the memory available and the memory used so far for the data array,with an HTML refresh of 10 seconds.
using the "/monitor" endpoint output the FIFO SensorData' array in reverse order as an HMTL table with an HTML refresh of 10 seconds
using the "/data" endpoint output to the client the entire FIFO SensorData' array as Json then clear the array.
using the "/data&reset" endpoint process as for teh non reset /Data endpoint, then reset the device.



# Mockups/Examples
None

## Example Input
No input

# Additional information

It has been noted by experiments that if the % of memory used for the 'sensorData' array is 50% of free memory then the device crashes - if you know why, please explain.
