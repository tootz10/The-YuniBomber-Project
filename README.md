# The-YuniBomber-Project
The YuniBomber reporsitory is an open-for-collaboration Multi Agent System project that consist of a serverscript(nodejs) and a clientscript(arduino). 

The YuniBomber reporsitory let you connect one or more Arduino Yun's to a nodejs server and play the YuniBomber game.

Gamerules:
- At start, the nodejs server deliver a virtuel bomb randomly to one of the connected clients
- After 3-5 minutes the time runs out and the Yun having the virtuel bomb looses.

## NodeServer.js - nodejs
The NodeServer.js file is, as the filename omplies, a nodejs server. NodeServer.js is used to handle the data communication between the connected Arduino Yuns.

Setup of Server;
Comming soon!

## YunClient - arduino
The YunClient folder includes a YunClient.ino file. The .ino can to uploaded to an Arduino Yun, to create connection between the to.

Guide to setup

Setup or Arduino/robot (Client side);
1. Connect the power to Arduino / robot, and access its wireless network with computer.
2. Set arduino to connect a shared network, for example a phone hotspot, that all robots and server connect to.
3. Connect the computer to the shared network.
4. Copy the folders from the "libraries" in the downloaded zip file to your local "libraries" folder under 'Documents' -> 'Arduino'
5. Open or restart the Arduino IDE, and ensure that the added libraries can be seen from the IDE, by clicking on
     'Sketch' -> 'Import Library' where the added libraries are supposed to occur
6. Open the file "YunClient" with Arduino IDE.
7. At line 52, edit the "IP" variable to the IP-address of your server.
8. Ensure that you are connected to the arduino through the IDE by clicking 'tools'->'port' where a wireless port for your arduino should be visiable
9. Upload the program to the arduino and open the serial monitor.
10. Once all players are connected the game will start automaticly. 

## Library
Optional! In this given project we have build three robots with a variety of sensors, that depend on corresponding libraries.
