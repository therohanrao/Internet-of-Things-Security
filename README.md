# Internet-of-Things-Security
IoT Project which runs on a Beagleboard and sends regular and encrypted messages (TCP and TLS) from board to a remote server. 
Receives real-time commands from the server and reacts by responding with real-time sensor data

By default, the program will report the time and temperature of its environment in the form:
hh:mm:ss fahrenheit

~~~
11:37:41 98.6
11:37:42 98.6
11:37:43 98.6
PERIOD=5
11:37:44 98.6
11:37:49 98.6
11:37:54 98.6
SCALE=C
11:37:59 37.0
11:38:04 37.0
STOP
START
11:38:19 37.0
11:38:24 37.0
OFF
11:38:27 SHUTDOWN
~~~
