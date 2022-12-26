#include <Arduino.h>
#include <MIDI.h>

#include <math.h>

#define DEBUG 0
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif 


#include <MonoPlayer.h>
#include <SequencerMono.h>
#include <Button.h>
USING_NAMESPACE_MIDI


//  thats basic functionality for monophonyc keyboard
//  it supports portamento as chromatic and linear with variable resolution to avchieve arpeggiated effects
//  it supports octave shifts from default you can move one oct down or two up
//  outputs as square wave to digital pin
//  yet to implement:
//  pitch bend
//  cv out
//  record and play sequences, ideally to external clock, should probably do as interrupt

//uint16_t frequencyChart[97];

// Setting the pins for scanning matrix and I/O
uint8_t outPins[] = {8, 7, 6, 5, 4, 3, 2};
uint8_t inPins[] = {9, 10, 11, 12, 14, 15, 16, 17};
const int outputPin = 13;

// chrPortaKey used for chromatic portamento
uint8_t numKeysPressed, pNumKeysPressed, chrPortaKey;

// Arrays that store all frequencyChart pressed in current and previous reading, empty keysPressed elements are zeros. PKeysPressed has frequencyChart sorted in it chronologically, with most recent key having greater index
// int keysPressed[8], PKeysPressed[8];

// portamento to be set as analog in: for now its const. 0 for no portamento
int portamentoMillis = 0;
// portaRes is resolution in millis for arpeggiated effects; set to 1 for no effect. also shoul be read from analog input
int portaRes = 20;

// set to true to go through a chromatic and false to go linearly through frequencies; will immideately play a half-tone without delay
bool chrPorta = true;
uint8_t octaveShift = 1;

unsigned long currentTime;

// this class plays without portamento

MonoPlayer *pMonoPlayer = new MonoPlayer();
Button *keys = new Button[44];


void ReadLoop()
{    
    for (uint8_t i = 0; i < 7; i++)
    {
        digitalWrite(outPins[i], LOW);
        for (uint8_t j = 0; j < 8; j++)
        {
            uint8_t key = i * 8 + j;
            if ((key>4) && (key<50)) keys[key].tick();
            
        }
        digitalWrite(outPins[i], HIGH);
    }
}

//wrappers
void _OnKeyPress(uint8_t    arg){
    
    pMonoPlayer->onKeyPress(arg);
    
}
void _OnKeyRelease(uint8_t    arg){
    pMonoPlayer->onKeyRelease(arg);
    
}
void setup()
{   
    Serial.begin(9600);
    
    debugln("Started setup");
    pinMode(outputPin, OUTPUT);
    digitalWrite(outputPin, LOW);
   
    // initialize matrix for scanning
    for (uint8_t i = 0; i < 7; i++)
    {
        pinMode(outPins[i], OUTPUT);
        digitalWrite(outPins[i], HIGH);
    }
    for (uint8_t i = 0; i < 8; i++)
    {
        pinMode(inPins[i], INPUT_PULLUP);
    }
    for (uint8_t i = 0; i < 7; i++)
    {
        
        for (uint8_t j = 0; j < 8; j++)
        {
            uint8_t key = i * 8 + j;
            keys[key].assignPin(inPins[j]);

            keys[key].attachOnPress( _OnKeyPress, key);
            keys[key].attachOnRelease( _OnKeyRelease, key);
            
        }
        
    }
    
        
       
    debugln("ended setup");

    
}
void loop()
{
    ReadLoop();
    pMonoPlayer->portamentoTick();
}

