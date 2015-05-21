# The-YuniBomber-Project
The YuniBomber reporsitory is an open-for-collaboration Multi Agent System project that consist of a serverscript(nodejs) and a clientscript(arduino). 

The YuniBomber reporsitory let you connect one or more Arduino Yun's to a nodejs server and play the YuniBomber game.

Gamerules:
- At start, the nodejs server deliver a virtuel bomb randomly to one of the connected clients
- After 3-5 minutes the time runs out and the Yun having the virtuel bomb looses.

## NodeServer.js
The NodeServer.js file is, as the filename omplies, a nodejs server. NodeServer.js is used to handle the data communication between the connected Arduino Yuns.

## YunClient
The YunClient folder includes a YunClient.ino file. The .ino can to uploaded to an Arduino Yun, to create connection between the to.

## Library
Optional! In this given project we have build three robots with a variety of sensors, that depend on corresponding libraries.
