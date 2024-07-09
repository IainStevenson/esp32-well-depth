# Project Overview

To produce a Visual Studio Code platformIO solution for an Expressif ESP-WROOM-32 device that satisfies the following requirements;



# Requirements

## Functional Requirements

1. Measure the current depth of water in a well via its 'known total depth' (assumed to be 2500 CM) minus its current level 'distance' below the sensor.
2. Calcualtes the sensors distance from the water level using its returned time value converted to CM, and subtracts that value from the known total depth value.
3. Stores the calculated depth data along with the exact UTC time it was taken in a memory array, with access to the array protected by a mutex
4. The in memory array is to be sized to use no more than 25% of the available memory.
5. the NNTP time values are to be storedas unsigned long but converted as String of format YYYY-MM-DD HH:MM:SS on output to any retrieval endpoint.6
6. Connects to the strongest WIFI signal available using the supplied MyWIFICreds.h file
7. Pauses and displays a 'Please reset' message if no WIFI endpoint is available.
8. If the network is lost the connection is retried without losing data
9. The data is gathered every 10 seconds.
10. The device can be queried asynchronously using HTTP on three endpoints, /reset/ data /monitor
11. No security requirements are required for the device.

## Non functional requirements
# Code files
1. The code can be spread across separate files if it makes it easier to understand and maintain.
2. Declare which Libraries need to be added to the project as part of the platformio.ini file
3. Assume Serial communications is the default and at a BAUD of 115200 

# User Stories
## User Story 1
As a [user], I want [power on the device] so that [it connects automatically to my network]
It should
Determine the best WIFI endpoint based on best RSSI.
Connect to that endpoint with teh credentials supplied
Connect to the NTTP time server and synchronise
Display the IP Address, MAC address and date ande time of the connection.

## User Story 2
As a [user], I want [data collected every 10 seconds and stored safely in an in memory FIFO array] so that [data can be accumulated for as long as possible before being lost].

## User Story 3
As a [user], I want [to query the http:/device-ip-address/reset end point] so that [the device resets itself] after return the respons that the reset has been accepted.

## User Story 4
As a [user], I want [to query the http:/device-ip-address/data end point] so that [the device returns the current contents of the array as JSON and clears the array].


## User Story 5
As a [user], I want [to query the http:/device-ip-address/monitor end point] so that [the device returns the current contents of the array in reverse chronological order as and HTML table DOES NOT clear the array].


# Technical Specifications
- Programming Language: [c++]
- Framework: [Arduino]
- Libraries: [To be specified]
- Tools: [VS Code/PlatformIO]

# Data Models
## Depth
- depth: float
- time: unsigned long 

# APIs
## Endpoint 1
- Method: GET
- URL: /data
- Request Parameters: [none]
- Response: [application/JSON]

## Endpoint 2
- Method: GET
- URL: /monitor
- Request Parameters: [none]
- Response: [text/HTML]

## Endpoint 2
- Method: GET
- URL: /reset
- Request Parameters: [none]
- Response: [text/HTML]


# Mockups/Examples
None
## Example Input
No input