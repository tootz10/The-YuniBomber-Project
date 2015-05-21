#include "Arduino.h"
#include "Position.h"

Position::Position(){
  X=0;
  Y=0;  
}    
    
Position::Position(int x, int y){
  X=x;
  Y=y;  
}
    
bool Position::checkForAdjent(Position b){ 
  if( (X<=(b.X+1)) && (X>=(b.X-1)) && (Y<=(b.Y+1)) && (Y>=(b.Y-1))){
    return true;
  }
  else{
    return false;
  }
}
    
bool Position::operator==(Position b){
  if(b.X==X && b.Y==Y){
    return true;
  }
  else{
    return false; 
  } 
}
