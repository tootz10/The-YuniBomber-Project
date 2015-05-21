var sys = require("sys"),
my_http = require("http");
var url = require('url');	

//*********PROTOCOL - TO SERVER**********
// /
// ID			(0) 0 = robot0, 1 = robot1, 2 = robot2
// CMD			(1) 0 = GET_ROBOT_DATA, 1 = SET_ROBOT_DATA, 2 = SEND_BOMB, 3 = GAME_STARTED_?
// SET_ROBOT_DATA
// xCoord		(2)
// yCoord		(3)
// GET_ROBOT_DATA
// SEND_BOMB
// toID			(2)

//*********PROTOCOL - FROM SERVER**********
// GET_ROBOT_DATA
// ID			(0)
// CMD			(1) = 0
// xCoord0		(2)
// yCoord0		(3)
// xCoord1		(4)
// yCoord1		(5)
// xCoord2		(6)
// yCoord2		(7)
// BombKeeper	(8)
// GAME_STARTED_?
// CMD			(0)	= 3
// STARTED		(1) 0 = NOT_STARTED, 1 = STARTED


var GET_ROBOT_DATA = "0";
var SET_ROBOT_DATA = "1";
var SEND_BOMB = "2";
var GAME_STARTED = "3";
var NO_BOMB = 0;
var HAVE_BOMB = 1;

var loggedIn = [0, 0, 0];

var robot0 = {xCoord: "0", yCoord: "0"};
var robot1 = {xCoord: "0", yCoord: "0"};
var robot2 = {xCoord: "0", yCoord: "0"};

var bombKeeper = Math.floor(Math.random() * 3);

my_http.createServer(function(request,response){
		
    sys.puts("I got kicked");
    response.writeHeader(200, {"Content-Type": "text/plain"});
	
	var path = url.parse(request.url).pathname;
	sys.puts(path);
	
	path = path.substr(1);
	
	sys.puts(path);
	
	switch(path.charAt(1)){
		case SET_ROBOT_DATA:
			switch(path.charAt(0)){
				case "0":
					robot0.xCoord = path.charAt(2);
					robot0.yCoord = path.charAt(3);
					sys.puts("Robot 0 position set to: (" + robot0.xCoord + "," + robot0.yCoord + ")");
					break;
				case "1":
					robot1.xCoord = path.charAt(2);
					robot1.yCoord = path.charAt(3);
					sys.puts("Robot 1 position set to: (" + robot1.xCoord + "," + robot1.yCoord + ")");
					break;
				case "2":	
					robot2.xCoord = path.charAt(2);
					robot2.yCoord = path.charAt(3);
					sys.puts("Robot 2 position set to: (" + robot2.xCoord + "," + robot2.yCoord + ")");
					break;
			}
			break;
		case GET_ROBOT_DATA:
			sys.puts("Sending Robot data to robot: " + path.charAt(0));
			response.write("0" + 
						   robot0.xCoord + robot0.yCoord +
						   robot1.xCoord + robot1.yCoord +
						   robot2.xCoord + robot2.yCoord +
						   bombKeeper);
						   
		sys.puts("Robot: 0, X: " + robot0.xCoord + ", Y: " + robot0.yCoord + ", Bomb: " + HaveBomb(0) + "\n" + 
				 "Robot: 1, X: " + robot1.xCoord + ", Y: " + robot1.yCoord + ", Bomb: " + HaveBomb(1) + "\n" +
				 "Robot: 2, X: " + robot2.xCoord + ", Y: " + robot2.yCoord + ", Bomb: " + HaveBomb(2) + "\n");					   
			
			break;
		case SEND_BOMB:
			bombKeeper = path.charAt(2);
			sys.puts("Robot " + path.charAt(0) + " gave the bomb to robot " + path.charAt(2));
			break;
		case GAME_STARTED:
			loggedIn[path.charAt(0)] = 1;
			sys.puts("Logged in bombers: {" + loggedIn[0] + ", " + loggedIn[1] + ", " + loggedIn[2] + "}");
			if(loggedIn[0] == 1 && loggedIn[1] == 1  && loggedIn[2] == 1){
				response.write("31");
				sys.puts("Game started");
			}
			else{
				response.write("30");
			}
			break;
		default:
		
			break;
	}
	
	response.end();		
	
}).listen(8080);
sys.puts("Server Running on 8080"); 

function HaveBomb(id){
	if(id == bombKeeper){
		return 1;
	}
	else{
		return 0;
	}
}