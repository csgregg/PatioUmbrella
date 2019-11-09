/*
The MIT License (MIT)
Copyright (c) 2016 Chris Gregg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-------------------------------------------------------------------------------------

Sets up PWM control for an LED. The constructor sets the pin output mode. The
PWM output is linearized.

The dim rate is the ammount the dim level of the LED is changed each time the
function autoDim() is called. Function autoDim() should be called by a timer, so
that there is a regular update level of the LED, regardless of the dim rate set.

Eg if autoDim() called every 50ms, LED will go from off to full in:
    - dimRate 5 : 1s (Flashing 2s cycle)
    - dimRate 10 : 500ms (Flashing 1s cycle)
    - dimRate 20 : 250ms (Flashing 500ms cycle)

Use function dimLED() to turn on and off dimming, and set if it is cyclick or just
stops at top or bottom of cycle.

Setting the on/off state with functions setState() or toggleState() does not effect
the dim level, just the on/of state.

Setting the level without the auto dimming, is done with function setLevel();

Level is a percentage.

*/

//#define DEBUG
#include "PWM_LED_control.h"


// Constructor
pwmLED::pwmLED( int outputPin, bool startState, int startLevel, int dimRate, bool dimUp, bool isCyclic )
{
  _outputPin = outputPin;         // Set output pin for this instance
  _outputLevel = startLevel;      // Set starting dim level
  _outputState = startState;      // Set starting state
  _dimRate = dimRate;             // Set starting dim rate
  _isCyclic = isCyclic;           // Set mode
  _dimUp =  dimUp;                // Set direction

  pinMode(_outputPin, OUTPUT);
}


// Get the current state
bool pwmLED::getState()
{
  return _outputState;
}


// Set state
void pwmLED::setState(bool newState)
{
  _isOverrun = false;
  
  if( newState != _outputState )          /// Do nothing if no change in state
  {
    _outputState = newState;

    if( _outputState ) this->setPinPWM( _outputLevel );     // Set on at stored level
    else this->setPinPWM( 0 );                              // Turn off 
  }
}


// Toggle state
void pwmLED::toggleState()
{
  bool newState = !_outputState;
  
  this->setState( newState );             // Toggle it
}


// Get the current level
int pwmLED::getLevel()
{
  return _outputLevel;
}


// Set level
void pwmLED::setLevel(int newLevel)
{
  if( newLevel != _outputLevel )              // Do nothing if no change to level
  {
    _outputLevel = newLevel;
    
    if( _outputState ) this->setPinPWM( _outputLevel );   // If on, then set it.
  }
}


// Update the pin PWM
void pwmLED::setPinPWM( int newLevel )
{
  int newOutputPWM = (int)(pow( (float)newLevel/(float)_PWM_LED_LEVEL_IN_MAX, 1/0.5 ) * (float)_PWM_MAX);       // Exponetial linearization

//  int newOutputPWM = (int)((float)_PWM_MAX/(1.0 + exp(-14.0*(((float)newLevel/50.0)/100))));                          // S-curve linearization

//  int newOutputPWM = map(newLevel, 0, _PWM_LED_LEVEL_IN_MAX, 0, _PWM_MAX);

  analogWrite( _outputPin, newOutputPWM );   // Set output

  DEBUG_PRINT("Pin ");
  DEBUG_PRINT( _outputPin );
  DEBUG_PRINT(" : Direction: ");
  DEBUG_PRINT( _dimUp );
  DEBUG_PRINT(", Overrun: ");
  DEBUG_PRINT( _isOverrun );
  DEBUG_PRINT(", State: ");
  DEBUG_PRINT( _outputState );
  DEBUG_PRINT(", Level: ");
  DEBUG_PRINT( newLevel );
  DEBUG_PRINT(", PWM: ");
  DEBUG_PRINTLN( newOutputPWM );
}


// Step to next auto dim level - typically called by a timer
void pwmLED::autoDim()
{
  if( !_outputState || !_dimLED || _isOverrun ) return;
 
  // Go up or down
  
  if( _dimUp ) _outputLevel += _dimRate;
  else _outputLevel -= _dimRate;

  // No over/under run
  if( _dimUp && _outputLevel >= _PWM_LED_LEVEL_IN_MAX )
  {
    _outputLevel = _PWM_LED_LEVEL_IN_MAX;
    _dimUp = !_isCyclic;
    _dimLED = _isCyclic;
    _isOverrun = !_isCyclic;
  }
  
  if( !_dimUp && _outputLevel <= 0 )
  {
    _outputLevel = 0; 
    _dimUp = _isCyclic;
    _dimLED = _isCyclic;
    _isOverrun = !_isCyclic;
    _outputState = _isCyclic;
  }

  this->setPinPWM( _outputLevel );   // Set it.
}


// Toggle dim direction
void pwmLED::toggleDimDirection()
{
  _isOverrun = false;
  _dimUp = !_dimUp;
}


// Set dim rate
void pwmLED::setDimRate(int dimRate)
{
  _dimRate = dimRate;
}


// Set dim direction
void pwmLED::setDimDirection(bool dimUp)
{
  _dimUp = dimUp;
}


// Dim the LED
void pwmLED::dimLED(bool startDimming)
{
  _dimLED = startDimming;
}

