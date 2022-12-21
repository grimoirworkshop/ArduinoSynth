#include <Arduino.h>

#include <math.h>

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
#include <MonoPlayer.h>
MonoPlayer *pMonoPlayer = new MonoPlayer();
class PianoKey
{
private:
    bool wasOn;

public:
    PianoKey()
    {
        wasOn = false;
    }
    void tick(bool trigger, uint8_t arg)
    {
        if (trigger)
        {
            if (!wasOn)
            {
                pMonoPlayer->onKeyPress(arg);
                wasOn = true;
                Serial.print("on ");
                Serial.println(arg);
            }
        }
        else if (wasOn)
        {
            pMonoPlayer->onKeyRelease(arg);
            wasOn = false;
            Serial.print("off ");
                Serial.println(arg);
        }
    };
};
PianoKey *keys =  new PianoKey[44];

// MonoPlayer pMonoPlayer;
//  reads out the matrix and stores it to keysPressed[]
//  also number of frequencyChart pressed

void ReadLoop()
{    
    for (uint8_t i = 0; i < 7; i++)
    {
        digitalWrite(outPins[i], LOW);
        for (uint8_t j = 0; j < 8; j++)
        {
            uint8_t key = i * 8 + j;
            if ((key > 4) && (key < 50)){
                keys[key].tick((digitalRead(inPins[j]) == LOW), key+24+octaveShift*12);
                
            }    
        }
        digitalWrite(outPins[i], HIGH);
    }
}
/*
void setTheFreq()
{
    for (uint8_t i = 29; i < 97; i++)
    {
        frequencyChart[i] = int(440.0 * pow(2.0, ((double(i) - 69) / 12)));
        Serial.println(frequencyChart[i]);
    };
}*/

void setup()
{
    Serial.begin(9600);
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

    // calculate values for frequencyChart[]
    //setTheFreq();
}
void loop()
{
    ReadLoop();
    pMonoPlayer->portamentoTick();
}

