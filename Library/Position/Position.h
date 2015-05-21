#ifndef Position_h
#define Position_h

#include "Arduino.h"

class Position{
  public:
    int X,Y;
    Position();
    Position(int x, int y);
    bool checkForAdjent(Position b);
    bool operator==(Position b);
    
  private:
};

#endif
