#include <Wire.h>
#include <Math.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <Console.h>
#include <Bridge.h>   
#include <HttpClient.h>

#include <Position.h>

class Color
{
  public:
    int R, G, B, C, X, Y;  

    Color(){
      R=0;
      G=0;
      B=0;
      C=0;
      X=0;
      Y=0;
    }
      
    Color(int r, int g, int b, int c, int x, int y){
      R=r;
      G=g;
      B=b;
      C=c;
      X=x;
      Y=y;
    }
 
    Position MatchColor(Color mapo[5][7], int tolerance) {     
      for(int i=0; i<5; i++){
        for(int j=0; j<7; j++){
          if( (R<(mapo[i][j].R+tolerance)) && (R>(mapo[i][j].R-tolerance)) && (G<(mapo[i][j].G+tolerance)) && (G>(mapo[i][j].G-tolerance)) && (B<(mapo[i][j].B+tolerance)) && (B>(mapo[i][j].B-tolerance)) && (C<(mapo[i][j].C+tolerance*2)) && (C>(mapo[i][j].C-tolerance*2))){
            return Position(i,j); 
          } 
        } 
      } 
      return Position(9,9);   
    }
    
    
};

/* Assign a unique ID to this sensor at the same time */
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);


String IP="192.168.1.220:8080";

//GLOBAL VARIABLES
  //Sonar
  #define trigPin 5
  #define echoPin 6
  const int pingPin = 7;
  long sonarDuration, inches, sonarDistance;
  long safetyDistance=5;
  long trackDistance=50;
  int sightDistance= 2;
  long bombDistance=10;
  //Compas
  int angleMargin;
  int goalAngle =0; 
  float currentAngle;
  int north=352;
  int east=75;
  int south=172;
  int west=269;
  int EdgeAvoidTrig=90;
  int EdgeNoAvoidTrig=100;
  //Bools
  bool turning;
  bool driving;
  bool avoiding;
  bool edgeavoid;
  //Motor
  int fullSpeed= 255;
  float motorCalibratorLeft = 1;
  float motorCalibratorRight = 1;
  //Color/Position
  Position currentPosition(9,9);
  Position lastEdgePosition(8,8);
  Position goalPosition(9,9);
  Position PlayerPositions[3];
  
  Color colorMap[5][7];
  Color tempColor;
  Color stopColor(17,22,15,60,0,0);
  
  //States
  int state;
  int noTarget;
  int yesTarget; 
  
  int searchCount=0;
  
  //Comunication
  HttpClient client;
  char dataString;
  bool getDataRdy;
  bool setDataRdy;
  int myId;
//  int myLastXPosition;
//  int myLastYPosition;
  int bombKeeper;
  int victim;
  bool gameStarted;
   
  byte i2cWriteBuffer[10];
  byte i2cReadBuffer[10];
  
  #define SensorAddressWrite 0x29 //
  #define SensorAddressRead 0x29 // 
  #define EnableAddress 0xa0 // register address + command bits
  #define ATimeAddress 0xa1 // register address + command bits
  #define WTimeAddress 0xa3 // register address + command bits
  #define ConfigAddress 0xad // register address + command bits
  #define ControlAddress 0xaf // register address + command bits
  #define IDAddress 0xb2 // register address + command bits
  #define ColorAddress 0xb4 // register address + command bits

void setup() {
  safetyDistance;
  angleMargin =15; 
  turning=false;
  driving=false;
  avoiding=false;
  edgeavoid=false;

  state=1;
  noTarget=1;
  yesTarget=1; 
  
  //Serial.begin(9600);  

  SetupMotors();
  SetupColor();
  SetupColorMap();
  SetupCompass();
  Break();
  
  Bridge.begin();
  Console.begin(); 

  while (!Console); 
  
  Console.println("Setup");
  //COMUNICATION
  myId = 1;
//  myLastXPosition = 5;
//  myLastYPosition = 5;
  gameStarted = false;
  getDataRdy = false;
  
  get_Position();
  SendData();
  GetData();
  //Forward(255);
  //driving=true;
}

int loops;
void loop() {
  //*******POSITION****
  get_Position();
  //*******COMPASS****
  currentAngle=ReadCompas();
  //********SONAR**********
  //sonarDistance=ReadSonar();
  sonarDistance=ReadSonar2();
  
  
  //*********Comunication*******
  /*if(loops%10==0){
    SendData();
    GetData();
    Console.println("Sending and getting data");
  }
  */
  
  //**********Print info********
  if(loops>=29){
    //Serial.print("sonarDistance= "); Serial.println(sonarDistance);
    //Console.print("sonarDistance= "); Console.println(sonarDistance);
    Console.print("goalAngle= "); Console.println(goalAngle);
    //Serial.print("Angle= "); Serial.println(currentAngle);
    Console.print(" Angle= "); Console.println(currentAngle);
    Console.print(" Position= "); Console.print(currentPosition.X); Console.println(currentPosition.Y);
    Console.print(" goalPosition= "); Console.print(goalPosition.X); Console.println(goalPosition.Y);
    loops=0;
    SendData();
    GetData();
    Console.println("Sending and getting data");
  }
  else{
    loops++;
  }
  
    
  //******Edge Avoidance*****
  if(( (currentPosition.Y==0&&EdgeAvoidTrig>=SmalestAngle( currentAngle, north)) || (currentPosition.Y==6&&EdgeAvoidTrig>=SmalestAngle( currentAngle, south)) || (currentPosition.X==0&&EdgeAvoidTrig>=SmalestAngle( currentAngle, west)) || (currentPosition.X==4&&EdgeAvoidTrig>=SmalestAngle( currentAngle, east)) ) && driving==true){
    //Console.print("Edge avoid ");
    Break();

    if(edgeavoid==false){
      if(currentAngle<=180){
        StartTurning(currentAngle+180, 255);
      }
      else{
        StartTurning(currentAngle-180, 255);       
      }
    }
    
    edgeavoid=true;
    driving=false;
    avoiding=false;
    turning=true;
  }
  
  if(((currentPosition.Y==0&&EdgeNoAvoidTrig<=SmalestAngle( currentAngle, north)) || (currentPosition.Y==6&&EdgeNoAvoidTrig<=SmalestAngle( currentAngle, south)) || (currentPosition.X==0&&EdgeNoAvoidTrig<=SmalestAngle( currentAngle, west)) || (currentPosition.X==4&&EdgeNoAvoidTrig<=SmalestAngle( currentAngle, east))) && avoiding==false &&edgeavoid==true && driving==false && turning==true){
    Break();
    edgeavoid=false;
    driving=false;
    avoiding=false;
    turning=false;
    //Console.print("driving off edge ");
  }
  
  //***Obstacle Avoidance***
  if(driving==true&&turning==false&&edgeavoid==false&&avoiding==false && sonarDistance<safetyDistance &&sonarDistance>0.1 ){
    Break();
    StartTurning(currentAngle-10, 255);
    avoiding=true;
    turning=true;
    driving=false;
    edgeavoid=false;
    //Console.print("obstacle Avoid ");
  }
  else if(driving==false&&avoiding==true&&turning==true &&sonarDistance>10){
    Break();
    avoiding=false;
    turning=false;
    edgeavoid=false;
    driving=true;
    //Console.print("Free of Obstacle ");
  }
  
  
  if(avoiding==false&&edgeavoid==false){
    switch(state){
      case 1://Init
        Console.println("initState1");
        gameStarted = GameStarted();
        SendData();
        delay(500);
        if(gameStarted){
          GetData();
          Console.print("Myid and bombId"); Console.print(myId); Console.print(" "); Console.print(bombKeeper);
          if(myId==bombKeeper){
            Console.println("got bombs");
            state=2; //bomb_t-
            noTarget=1;//find
            yesTarget=1;//chase
          }
          else{
            state=4; //evade
            noTarget=1;//find
            yesTarget=1;//chase 
          }
        }
        
        break;
        
      case 2://bomb_t-
        switch(noTarget){
          case 1://Find
            Console.println("Find state ");
            //Find closest opponent
            float distances[2];
            float closestDist;
            closestDist=10000000;
            distances[0] = sqrt(abs(PlayerPositions[0].X-currentPosition.X)*abs(PlayerPositions[0].X-currentPosition.X)+abs(PlayerPositions[0].Y-currentPosition.Y)*abs(PlayerPositions[0].Y-currentPosition.Y));
            distances[1] = sqrt(abs(PlayerPositions[1].X-currentPosition.X)*abs(PlayerPositions[1].X-currentPosition.X)+abs(PlayerPositions[1].Y-currentPosition.Y)*abs(PlayerPositions[1].Y-currentPosition.Y));
            distances[2] = sqrt(abs(PlayerPositions[2].X-currentPosition.X)*abs(PlayerPositions[2].X-currentPosition.X)+abs(PlayerPositions[2].Y-currentPosition.Y)*abs(PlayerPositions[2].Y-currentPosition.Y));
              
            for(int i=0; i<3; i++){
              if(!(PlayerPositions[i]==currentPosition)){
                if(distances[i]<closestDist){
                  if(PlayerPositions[victim].X<5&&PlayerPositions[victim].X>=0&&PlayerPositions[victim].Y<7&&PlayerPositions[victim].Y>=0){
                    closestDist=distances[i];
                    victim=i;  
                  }
                }                
              }  
            }
            Console.print(" OFFERLAM!: ");Console.println(victim);
            
            goalPosition.X = PlayerPositions[victim].X;
            goalPosition.Y = PlayerPositions[victim].Y;  
            goalAngle=calculate_goalAngle();
            
            //if opponent is within sightDistance, scan with sonar, if not fo to "relocate"-state
              if(distances[victim]>sightDistance){
                    state=2;
                    noTarget=2;//relocate
                    yesTarget=1;//chase
                    SendData();
                    GetData();
              }
              else{
                Console.println("Scanning ");
                if(sonarDistance<trackDistance&&sonarDistance>0.1 && (currentAngle>(goalAngle-angleMargin*3)) && (currentAngle<(goalAngle+angleMargin*3))){
                  Console.println("Target Found");
                  Break();
                  Forward(255);
                  noTarget=1;//find
                  yesTarget=1;//chase
                  state=3;//bomb_t+
                  searchCount=0;
                }
                else{
                  StartTurning(currentAngle+1, 255);  
                }
              } 
            break;
            
          case 2://relocate
            Console.println("relocate");
            goalPosition.X = PlayerPositions[victim].X;
            goalPosition.Y = PlayerPositions[victim].Y; 
            goalAngle = calculate_goalAngle();
            
            //if within sightrange go bakc to "find"-state, if not, drive toward victim
            if(sqrt(abs(PlayerPositions[victim].X-currentPosition.X)*abs(PlayerPositions[victim].X-currentPosition.X)+abs(PlayerPositions[victim].Y-currentPosition.Y)*abs(PlayerPositions[victim].Y-currentPosition.Y))<sightDistance){
              Console.println("within sightRange");
              noTarget=1;//find
              yesTarget=1;//chase
              state=2;//bomb_t-
            }
            else if((currentAngle>(goalAngle-angleMargin)) && (currentAngle<(goalAngle+angleMargin))){
              Forward(255);  
            }
            else{
              StartTurning(goalAngle, 255);
            }
            break;
        }
        break;
        
      case 3://bomb_t+
        switch(yesTarget){
          case 1://chase
            Console.println("chasing");
            Forward(255); 
            //if victim is seen by sonar, drive forward, if sight lost go back to "find"-state
            if(sonarDistance>trackDistance&&sonarDistance>0.1)
            {
              noTarget=1;//find
              yesTarget=1;//chase
              state=2;//bomb_t-0
            }
            else if(sonarDistance<=bombDistance&&sonarDistance>0.1){
              noTarget=1;//find
              yesTarget=2;//pass_bomb
              state=3;//bomb_t+s
            }
            break;
            
          case 2://pass_bomb
            noTarget=1;//find
            yesTarget=1;//chase
            state=4;//bomb_t-
            Console.println("BOMBPASSED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            GiveBomb(victim);
            SendData();
            GetData();
            Break();
            break;
        }
        break;
        
      case 4://evade
        
        //drive opposite direction of bomber
        Console.println("evading");
        //if(( (currentPosition.Y==0&&EdgeAvoidTrig>=SmalestAngle( currentAngle, north)) || (currentPosition.Y==6&&EdgeAvoidTrig>=SmalestAngle( currentAngle, south)) || (currentPosition.X==0&&EdgeAvoidTrig>=SmalestAngle( currentAngle, west)) || (currentPosition.X==4&&EdgeAvoidTrig>=SmalestAngle( currentAngle, east)) )){
        //  Break();
        //}
        //else
        if( (currentPosition.Y==0) || (currentPosition.Y==6) || (currentPosition.X==0) || (currentPosition.X==4) ){
          Forward(255);
        }
        else{
          goalPosition = PlayerPositions[bombKeeper];
          goalAngle = calculate_goalAngle()-180;
          if(goalAngle<0)
            goalAngle+=360;
            
          if((currentAngle>(goalAngle-angleMargin)) && (currentAngle<(goalAngle+angleMargin)))
            Forward(255);
          else{
            StartTurning(goalAngle, 255);
          }
        }
        
        if(bombKeeper==myId){
          noTarget=1;//find
          yesTarget=1;//pass_bomb
          state=2;//bomb_t+s
        }
        
        break;
        
    }
  }
    /*    
    if(currentPosition==goalPosition&&turning==false&&targetLock==false){
      turning=true;
      driving=false;
      edgeavoid=false;
      avoiding=false;
      StartTurning(currentAngle+10, 255);  
      Console.print("Scanning ");
    }
    
    if(turning==true&&driving==false&&edgeavoid==false&&avoiding==false&&sonarDistance<trackDistance&&sonarDistance>0.1){
      Console.println("Target Found");
      avoiding=false;
      edgeavoid=false;
      driving=true;
      turning=false;
      targetLock=true;
      Break();
      Forward(255);
    }
    
    //***Turn towards goal angle***
    /*if(((currentAngle<(goalAngle-angleMargin))||(currentAngle>(goalAngle+angleMargin)))&&turning==false && avoiding==false){
      StartTurning(goalAngle, 255);
      turning=true;
      Console.print("5 ");
    }
    else if((currentAngle>(goalAngle-angleMargin)) && (currentAngle<(goalAngle+angleMargin)) && avoiding==false &&edgeavoid==true){
      Break();
      Forward(255);
      driving=true;
      turning=false;
      Console.print("Turning at edge ");
    }
    */
}
////////////////////////////////////
//comunicationSCRIPS
///////////////////////////////////
void SendData(){
  String dataString = String("/") + myId + 1 + currentPosition.X + currentPosition.Y;
  //Console.println("Send: " + dataString);
  client.get(IP + dataString);
}

void GetData(){
  String cmdString = String("/") + myId + 0;  // "/00"
  //Console.println("Get: " + cmdString);
  client.get(IP + cmdString);
  
  String dataString = String("");
  while (client.available()) { //Hent data så længe der er noget
    char val = client.read(); 
    dataString += val;        //Læg data i streng
  }
  
  int tempStatus=bombKeeper;
  //Console.println(dataString);

  PlayerPositions[0].X = dataString.charAt(1) - '0'; //Char at 1 to number
  PlayerPositions[0].Y = dataString.charAt(2) - '0'; //Char at 2 to number
  PlayerPositions[1].X = dataString.charAt(3) - '0'; //Char at 3 to number
  PlayerPositions[1].Y = dataString.charAt(4) - '0'; //Char at 4 to number
  PlayerPositions[2].X = dataString.charAt(5) - '0'; //Char at 5 to number
  PlayerPositions[2].Y = dataString.charAt(6) - '0'; //Char at 6 to number
  bombKeeper = dataString.charAt(7) - '0'; //Char at 7 to number
  Console.print(" BOMBEMANDEN ER: ");Console.println(bombKeeper);
  //if we just got momb, wait 2 sec
  if(tempStatus!=bombKeeper&&bombKeeper==myId)
    delay(4000);
    
}

bool GameStarted(){
  
  //Console.println("Checking if started");
  bool gameStarted = false;
  
  String cmdString = String("/") + myId + 3;
  client.get(IP + cmdString);
  
  String dataString = String("");  
  while (client.available()) {                    // Read message from server
    char val = client.read();                    // Val is a char and 1 is = 49
    dataString += val;                          // 
  }
  
  if(dataString.charAt(1) - '0' == 1){
    gameStarted = true;  
  }
  
  return gameStarted;
}

void GiveBomb(int targetId){
  String dataString = String("/") + myId + 2 + targetId;
  client.get(IP + dataString);
}


////////////////////////////////////
//MOTORSCRIPS
///////////////////////////////////
void StartTurning(int goalAngleLocal, int pwm){
  //Console.print("PWM "); Console.println(pwm);
  driving=false;
  if(abs(goalAngleLocal-currentAngle)<180){
    if(goalAngleLocal-currentAngle>0){
      digitalWrite(13, LOW); //Establishes forward direction of Channel B
      digitalWrite(8, LOW);   //Disengage the Brake for Channel B
      analogWrite(11, pwm*motorCalibratorLeft);   //Spins the motor on Channel B
      
      digitalWrite(12, HIGH); //Establishes backward direction of Channel A
      digitalWrite(9, LOW);   //Disengage the Brake for Channel A
      analogWrite(3, pwm*motorCalibratorRight);   //Spins the motor on Channel A
    }
    else{
      digitalWrite(13, HIGH); //Establishes Backward direction of Channel B
      digitalWrite(8, LOW);   //Disengage the Brake for Channel B
      analogWrite(11, pwm*motorCalibratorLeft);   //Spins the motor on Channel B
      
      digitalWrite(12, LOW); //Establishes forkward direction of Channel A
      digitalWrite(9, LOW);   //Disengage the Brake for Channel A
      analogWrite(3, pwm*motorCalibratorRight);   //Spins the motor on Channel A      
    }
  }
  else{
    if(goalAngleLocal-currentAngle<0){
      digitalWrite(13, LOW); //Establishes forward direction of Channel B
      digitalWrite(8, LOW);   //Disengage the Brake for Channel B
      analogWrite(11, pwm*motorCalibratorLeft);   //Spins the motor on Channel B
      
      digitalWrite(12, HIGH); //Establishes backward direction of Channel A
      digitalWrite(9, LOW);   //Disengage the Brake for Channel A
      analogWrite(3, pwm*motorCalibratorRight);   //Spins the motor on Channel A
    }
    else{
      digitalWrite(13, HIGH); //Establishes Backward direction of Channel B
      digitalWrite(8, LOW);   //Disengage the Brake for Channel B
      analogWrite(11, pwm*motorCalibratorLeft);   //Spins the motor on Channel B
      
      digitalWrite(12, LOW); //Establishes forkward direction of Channel A
      digitalWrite(9, LOW);   //Disengage the Brake for Channel A
      analogWrite(3, pwm*motorCalibratorRight);   //Spins the motor on Channel A      
    }
  }
  
}

void Break(){
    driving=false;
    digitalWrite(9, HIGH);  //Engage the Brake for Channel A
    analogWrite(3, 255);   //Spins the motor on Channel A 
    
    digitalWrite(8, HIGH);  //Engage the Brake for Channel B
    analogWrite(11, 255);   //Spins the motor on Channel B
}

void Forward(float pwm){
      driving=true;
      digitalWrite(13, LOW); //Establishes forward direction of Channel B
      digitalWrite(8, LOW);   //Disengage the Brake for Channel B
      analogWrite(11, pwm*motorCalibratorLeft);   //Spins the motor on Channel B
      
      digitalWrite(12, LOW); //Establishes backward direction of Channel A
      digitalWrite(9, LOW);   //Disengage the Brake for Channel A
      analogWrite(3, pwm*motorCalibratorRight);   //Spins the motor on Channel A
}

void Backward(float pwm){
      driving=true;
      digitalWrite(13, HIGH); //Establishes forward direction of Channel B
      digitalWrite(8, LOW);   //Disengage the Brake for Channel B
      analogWrite(11, pwm*motorCalibratorLeft);   //Spins the motor on Channel B
      
      digitalWrite(12, HIGH); //Establishes backward direction of Channel A
      digitalWrite(9, LOW);   //Disengage the Brake for Channel A
      analogWrite(3, pwm*motorCalibratorRight);   //Spins the motor on Channel A 
}

/*
void TestRun(){ 
  delay(1000);
  Forward(255);

  delay(3000);
  
  
  analogWrite(3, 255);   //Spins the motor on Channel A at full speed
  analogWrite(11, 255);   //Spins the motor on Channel A at full speed
  digitalWrite(9, HIGH);  //Engage the Brake for Channel A 
  digitalWrite(8, HIGH);  //Engage the Brake for Channel B
}
*/
void SetupMotors(){
  //Setup Channel A
  pinMode(12, OUTPUT); //Initiates Motor Channel A pin
  pinMode(9, OUTPUT); //Initiates Brake Channel A pin

  //Setup Channel B
  pinMode(13, OUTPUT); //Initiates Motor Channel B pin
  pinMode(8, OUTPUT);  //Initiates Brake Channel B pin
}
/*
void CalibrateMotors(){ 
  float angleRatio;
  
  //delay(2000);
  float motorAngleLeft = CalibrateLeftMotor();
  delay(2000);
  float motorAngleRight = CalibrateRightMotor();
  
  if(motorAngleRight >= motorAngleLeft){
    angleRatio = motorAngleLeft / motorAngleRight;
    
    motorCalibratorRight = angleRatio;
    motorCalibratorLeft = 1;
  }
  else{
    angleRatio = motorAngleRight / motorAngleLeft;
    
    motorCalibratorRight = 1;
    motorCalibratorLeft = angleRatio;
  }
  
}

float CalibrateLeftMotor(){
  //READ AND SAVE COMPASS DATA
  
  //Motor A Brake
  digitalWrite(12, LOW); //Establishes forward direction of Channel A
  digitalWrite(9, HIGH);   //Engage the Brake for Channel A
  analogWrite(3, 255);   //Spins the motor on Channel A at full speed

  //Motor B forward @ half speed
  digitalWrite(13, LOW);  //Establishes forward direction of Channel B
  digitalWrite(8, LOW);   //Disengage the Brake for Channel B
  analogWrite(11, 128);    //Spins the motor on Channel B at half speed
  
  float  first = ReadCompas();
  
  delay(2000);
  
  float  second = ReadCompas();
  
  //READ AND SAVE COMPASS DATA
  digitalWrite(8, HIGH);  //Engage the Brake for Channel B 

  if(second-first<0)
    return abs(second+360-first);
  else
    return abs(second-first); //RETURN COMPASS DATA
}

float CalibrateRightMotor(){  
  //READ AND SAVE COMPASS DATA
  
  //Motor A forward @ half speed
  digitalWrite(12, LOW); //Establishes forward direction of Channel A
  digitalWrite(9, LOW);   //Disengage the Brake for Channel A
  analogWrite(3, 128);   //Spins the motor on Channel A at half speed

  //Motor B Brake
  digitalWrite(13, LOW);  //Establishes forward direction of Channel B
  digitalWrite(8, HIGH);   //Engage the Brake for Channel B
  analogWrite(11, 255);    //Spins the motor on Channel B at full speed
  
  float  first = ReadCompas();
  
  delay(2000);
  
  float  second = ReadCompas();
  
  //READ AND SAVE COMPASS DATA
  digitalWrite(9, HIGH);  //Engage the Brake for Channel A 
  
  if(second-first>0)
    return abs(second-first+360);
  else
    return abs(second-first); //RETURN COMPASS DATA
}
*/
////////////////////////////////////
//COMPASSENSORSCRIPS
///////////////////////////////////
void SetupCompass(){
      // Initialise the Compas
  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    //Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }
}

float ReadCompas(){
  sensors_event_t event; 
  mag.getEvent(&event);
  
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  float declinationAngle = 0.04;
  heading -= declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI;
  
  return headingDegrees;

}

int SmalestAngle(int first, int second){
  int dif =abs(first-second)%360; 
  
  if (dif > 180)
    dif = 360 - dif;
    
  return dif;
}

/*
////////////////////////////////////
//SONARSENSORSCRIPS
///////////////////////////////////
long ReadSonar(){ //3pin version
  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  sonarDuration = pulseIn(pingPin, HIGH, 10000);

  // convert the time into a distance
  return microsecondsToCentimeters(sonarDuration);  
}
*/
long ReadSonar2(){
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
    long duration, distance;
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH, 10000);
  distance = (duration/2) / 29.1;
    //Console.print(distance);
    ////////Console.println(" cm");
  return distance;
}

long microsecondsToCentimeters(long microseconds){
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the sonarDistance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

////////////////////////////////////
//COLORSENSORSCRIPS
///////////////////////////////////
void SetupColor() {
  Wire.begin();
  init_TCS34725();
  get_TCS34725ID();     // get the device ID, this is just a test to see if we're connected
}

void SetupColorMap(){
  // Colormapping
  //colorMap[0][0]= Color(17,22,15,60,0,0);
  colorMap[0][0]= Color (108,35,30,169,0,0);
  colorMap[1][0]= Color (342,394,227,511,4,0);
  colorMap[2][0]= Color (273,149,60,511,2,0);
  colorMap[3][0]= Color (277,199,152,511,3,0);
  colorMap[4][0]= Color (165,57,42,272,1,0);
  
  colorMap[0][1]= Color (153,49,41,245,0,1);
  colorMap[1][1]= Color (263,113,49,449,1,1);
  colorMap[2][1]= Color (298,237,87,511,2,1);
  colorMap[3][1]= Color (247,162,126,511,3,1);
  colorMap[4][1]= Color (319,306,218,511,4,1);
  
  colorMap[0][2]= Color (336,298,83,511,0,2);
  colorMap[1][2]= Color (200,218,182,511,3,2);
  colorMap[2][2]= Color (345,376,160,511,2,2);
  colorMap[3][2]= Color (318,310,108,511,1,2);
  colorMap[4][2]= Color (262,295,222,511,4,2);
  
  colorMap[0][3]= Color (123,208,71,449,0,3);
  colorMap[1][3]= Color (160,275,129,511,1,3);
  colorMap[2][3]= Color (215,336,195,511,2,3);
  colorMap[3][3]= Color (190,303,216,511,3,3);
  colorMap[4][3]= Color (91,105,113,333,4,3);
  
  colorMap[0][4]= Color (31,55,29,127,0,4);
  colorMap[1][4]= Color (93,191,91,417,1,4);
  colorMap[2][4]= Color (222,320,169,511,2,4);
  colorMap[3][4]= Color (87,200,170,511,3,4);
  colorMap[4][4]= Color (53,144,137,373,4,4);
  
  colorMap[0][5]= Color (55,41,24,128,0,5);
  colorMap[1][5]= Color (49,103,47,221,1,5);
  colorMap[2][5]= Color (172,291,216,511,2,5);
  colorMap[3][5]= Color (121,251,204,511,3,5);
  colorMap[4][5]= Color (45,114,125,314,4,5);
  
  //colorMap[5][5]= Color (23,31,39,93,5,5); Mørkeblå
  colorMap[0][6]= Color (37,73,90,218,5,4);
  colorMap[1][6]= Color (84,70,86,250,5,3);
  colorMap[2][6]= Color (197,234,146,511,5,2);
  colorMap[3][6]= Color (163,203,132,511,5,1);
  colorMap[4][6]= Color (324,382,263,511,5,0); 
}

void Writei2cRegisters(byte numberbytes, byte command){
    byte i = 0;

    Wire.beginTransmission(SensorAddressWrite);   // Send address with Write bit set
    Wire.write(command);                          // Send command, normally the register address 
    for (i=0;i<numberbytes;i++)                       // Send data 
      Wire.write(i2cWriteBuffer[i]);
    Wire.endTransmission();

    delayMicroseconds(100);      // allow some time for bus to settle      
}

/*  
Send register address to this function and it returns byte value
for the magnetometer register's contents 
*/
byte Readi2cRegisters(int numberbytes, byte command){
   byte i = 0;

    Wire.beginTransmission(SensorAddressWrite);   // Write address of read to sensor
    Wire.write(command);
    Wire.endTransmission();

    delayMicroseconds(100);      // allow some time for bus to settle      

    Wire.requestFrom(SensorAddressRead,numberbytes);   // read data
    for(i=0;i<numberbytes;i++)
      i2cReadBuffer[i] = Wire.read();
    Wire.endTransmission();   

    delayMicroseconds(100);      // allow some time for bus to settle      
}  

void init_TCS34725(void){
  i2cWriteBuffer[0] = 0x10;
  Writei2cRegisters(1,ATimeAddress);    // RGBC timing is 256 - contents x 2.4mS =  
  i2cWriteBuffer[0] = 0x00;
  Writei2cRegisters(1,ConfigAddress);   // Can be used to change the wait time
  i2cWriteBuffer[0] = 0x00;
  Writei2cRegisters(1,ControlAddress);  // RGBC gain control
  i2cWriteBuffer[0] = 0x03;
  Writei2cRegisters(1,EnableAddress);    // enable ADs and oscillator for sensor  
}

void get_TCS34725ID(void){
  Readi2cRegisters(1,IDAddress);
  if (i2cReadBuffer[0] = 0x44)
    Console.println("TCS34725 is present");    
  else
    Console.println("TCS34725 not responding");    
}
/*

Reads the register values for clear, red, green, and blue.
*/
void get_Colors(void){
  unsigned int clear_color = 0;
  unsigned int red_color = 0;
  unsigned int green_color = 0;
  unsigned int blue_color = 0;

  Readi2cRegisters(8,ColorAddress);
  clear_color = ((unsigned int)(i2cReadBuffer[1]<<8) + (unsigned int)i2cReadBuffer[0])/128;
  red_color = ((unsigned int)(i2cReadBuffer[3]<<8) + (unsigned int)i2cReadBuffer[2])/128;
  green_color = ((unsigned int)(i2cReadBuffer[5]<<8) + (unsigned int)i2cReadBuffer[4])/128;
  blue_color = ((unsigned int)(i2cReadBuffer[7]<<8) + (unsigned int)i2cReadBuffer[6])/128;

  // send register values to the serial monitor 
  /*
  Console.print(" R=");
  Console.print(red_color, DEC);    
  Console.print(" G=");
  Console.print(green_color, DEC);    
  Console.print(" B=");
  Console.print(blue_color, DEC);
  Console.print(" C=");
  Console.println(clear_color, DEC); 
  */
  
  tempColor= Color((int)red_color ,(int)green_color ,(int)blue_color ,(int)clear_color ,9 ,9);
  
 // Basic RGB color differentiation can be accomplished by comparing the values and the largest reading will be 
 // the prominent color
/*
  if((red_color>blue_color) && (red_color>green_color))
    Serial.println("detecting red");
  else if((green_color>blue_color) && (green_color>red_color))
    Serial.println("detecting green");
  else if((blue_color>red_color) && (blue_color>green_color))
    Serial.println("detecting blue");
  else
    Serial.println("color not detectable");
*/
}  

////////////////////////////////////
//POSITIONSCRIPS
///////////////////////////////////
void get_Position(){
  Position tempPosition;
  
  get_Colors();
  
  tempPosition=tempColor.MatchColor(colorMap, 15);
  //Serial.print(tempPosition.X);
  //Serial.print(" ");
  //Serial.print(tempPosition.Y);
  //Console.print(" ");
  //Console.print(tempPosition.X);
  //Console.print(" ");
  //Console.print(tempPosition.Y);
  
  if(tempPosition.X!=9 && tempPosition.Y!=9){  
    //if(currentPosition.X==9){
    currentPosition=tempPosition;
   // }
    //else if(tempPosition==currentPosition){
      //Serial.println(" Same pos ");
     // Console.println(" Same pos ");
    //}
    //else //BEWARE! NO CHECK FOR ADJENT //if(currentPosition.checkForAdjent(tempPosition)){
      //currentPosition=tempPosition;  
      //Serial.println(" Changed pos ");
     // Console.println(" Changed pos ");
    //}
  }
  else {
      //Serial.println(" Position not detected ");
     // Console.println(" Position not detected ");
  }
}

int calculate_goalAngle(){
  
  double deltay=goalPosition.Y-currentPosition.Y;
  double deltax=goalPosition.X-currentPosition.X;
  double angle= atan2(deltay, deltax)*180/PI;
  
  angle+=east+(360-north);
  
  if(angle<0)
    angle+=360;
  else if(angle>360)
    angle-=360;
    
  return (int) angle;
}
