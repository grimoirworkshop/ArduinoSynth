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


uint16_t frequencyChart[79];

// Setting the pins for scanning matrix and I/O
uint8_t outPins[] = {8, 7, 6, 5, 4, 3, 2};
uint8_t inPins[] = {9, 10, 11, 12, 14, 15, 16, 17};
const int outputPin = 13;








// chrPortaKey used for chromatic portamento
uint8_t numKeysPressed, pNumKeysPressed, chrPortaKey;

// Arrays that store all frequencyChart pressed in current and previous reading, empty array elements are zeros. PKeysPressed has frequencyChart sorted in it chronologically, with most recent key having greater index
//int keysPressed[8], PKeysPressed[8];

// portamento to be set as analog in: for now its const. 0 for no portamento
int portamentoMillis = 0;
// portaRes is resolution in millis for arpeggiated effects; set to 1 for no effect. also shoul be read from analog input
int portaRes = 20;

// set to true to go through a chromatic and false to go linearly through frequencies; will immideately play a half-tone without delay
bool chrPorta = true;

 



unsigned long currentTime;

//this class plays without portamento
class MonoPlayer{
    private : 
        
         uint8_t octaveShift; //has range 0-3; should check it when changigng 
         float currentFr;        
         uint16_t goalFr;
         uint8_t keyPlayed;
         float step;
         unsigned long portamentoStartTime;
         enum {silent, noteOnHold, inPortameno, inChrPortamento} states;    
         uint8_t portamento;
         uint8_t    portamentoTickTime;
         bool chromaticPortamento;
         bool playFromMIDI;
         bool sendMIDI;         
         uint8_t MIDIchannel;
         uint8_t keysPressed[8];        
         float bend;
    public:   
        
        MonoPlayer(){
            step = 0.0;            
            playFromMIDI = false;
            sendMIDI = false;            
            portamento = 0;
            portamentoTickTime = 1;
            chromaticPortamento = false;
            octaveShift = 1;
            states = silent;          
        }; 

        void onKeyPress(uint8_t key){
            key += (octaveShift*12);
            
            goalFr = (frequencyChart[key])*bend; 
            switch(states){
                case silent:
                    noteOn(key);
                    keysPressed[0] = key;
                    break;
                case noteOnHold:
                    if (portamento == 0){

                    }
                default:
                    //add to keysPressed array
                    setPortamento(key);
                    break;             
            }
        }
         void onKeyRelease(uint8_t key){
            for (int i = 0; i<8; i++){

            }

        }
         void setPortamento(uint8_t key){

        }
         void noteOn(uint8_t key){
            tone(outputPin, goalFr);
            currentFr = goalFr;           
            states = noteOnHold;
            keyPlayed = key;                        
        };         
         void stop(){
            noTone(outputPin);
            states = silent;
            
        }; 
        
         bool needToTick(unsigned long portaPreviousTime){
            if ((millis()-portaPreviousTime) >= portamentoTickTime){
                return true;
            }
            else {return false;}
        };
        
};
MonoPlayer * pMonoPlayer = new MonoPlayer();
class MatrixKey{
    private:   
        bool wasOn;
        
    public:        
        
        MatrixKey(){
            wasOn = false;
            
        }  
        void tick(bool trigger, uint8_t arg){
            if (trigger){
                if (!wasOn){
                    pMonoPlayer->onKeyPress(arg);
                    wasOn = true;
                }
            }
            else if (wasOn){
                pMonoPlayer->onKeyRelease(arg);
                wasOn = false;                
            }
        };
        

                
};
MatrixKey keys[44];


//MonoPlayer pMonoPlayer;
// reads out the matrix and stores it to keysPressed[]
// also number of frequencyChart pressed

void ReadLoop()
{
    
    uint8_t i, j;
    pNumKeysPressed = numKeysPressed;
    numKeysPressed = 0;
    for (i = 0; i < 7; i++)
    {
        digitalWrite(outPins[i], LOW);
        for (j = 0; j < 8; j++)
        {
            keys[i*8+j].tick((digitalRead(inPins[j]) == LOW), i*8+j);
        }
        digitalWrite(outPins[i], HIGH);
    }
}

void setTheFreq()
{
    for (uint8_t i=29; i<79; i++){
        frequencyChart[i] =int(440.0 * pow(double(i), ((double(i)-69)/12)));
        Serial.println(frequencyChart[i]);
    };    
}

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
    setTheFreq();

    
}
void loop()
{
    ReadLoop();   
    
}
