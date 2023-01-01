#include <Arduino.h>


#include <functionPrototypes.h>



#define DEBUG 0
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif 




//  thats basic functionality for monophonyc keyboard
//  it supports portamento as chromatic and linear with variable resolution to avchieve arpeggiated effects
//  it supports octave shifts from default you can move one oct down or two up
//  outputs as square wave to digital pin
//  yet to implement:
//  pitch bend
//  cv out
//  record and play sequences, ideally to external clock, should probably do as interrupt


// Setting the pins for scanning matrix and I/O
uint8_t outPins[] = {8, 7, 6, 5, 4, 3, 2};
uint8_t inPins[] = {9, 10, 11, 12, 14, 15, 16, 17};
const int outputPin = 13;
uint8_t numKeysPressed, pNumKeysPressed;
uint8_t octaveShift = 1;
unsigned long currentTime;


#include <MonoPlayer.h>
#include <SequencerMono.h>
#include <Button.h>


MonoPlayer *pMonoPlayer = new MonoPlayer();
Button *keys = new Button[44];




void setup()
{   
    Serial.begin(9600);
    while (!Serial);
     
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

void ReadLoop( )
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


void MIDIsendNoteOn(byte note, byte velocity, byte channel ){
Serial.write(144);
Serial.write(note);
Serial.write(velocity);

}

//wrappers

void _OnKeyPress(uint8_t    arg){
    
    pMonoPlayer->onKeyPress(arg);
    
}
void _OnKeyRelease(uint8_t    arg){
    pMonoPlayer->onKeyRelease(arg);
    
}



#include <functions.h>
