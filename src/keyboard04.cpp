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
class MatrixKey{
    private:   
        bool wasOn;
        
    public:        
        void (*onPress)(uint8_t);
        void (*onHold)(uint8_t);
        void (*onRelease)(uint8_t);
        MatrixKey(){
            wasOn = false;
            
        }  
        void tick(bool trigger, uint8_t arg){
            if (trigger){
                if (wasOn){
                    onHold(arg);
                }
                else {
                    onPress(arg);
                    wasOn = true;
                }
            }
            else if (wasOn){
                onRelease(arg);                
            }
        };
        

                
};
//this class plays without portamento
class MonoPlayer{
    private : 
        
        static uint8_t octaveShift; //has range 0-3; should check it when changigng 
        static float currentFr;        
        static uint16_t goalFr;
        static uint8_t keyPlayed;
        static float step;
        static unsigned long portamentoStartTime;
        static enum {silent, noteOnHold, inPortameno, inChrPortamento} states;    
        static uint8_t portamento;
        static int8_t    portamentoTickTime;
        static bool chromaticPortamento;
        static bool playFromMIDI;
        static bool sendMIDI;         
        static uint8_t MIDIchannel;
        static uint8_t keysPressed[8];        
        static float bend;
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

        static void onKeyPress(uint8_t key){
            key += (octaveShift*12);
            
            goalFr = (frequencyChart[key])*bend; 
            switch(states){
                case silent:
                    noteOn(key);
                    keysPressed[0] = key;
                    break;
                case noteOnHold:
                    if (portamento = 0){

                    }
                default:
                    //add to keysPressed array
                    setPortamento(key);
                    break;             
            }
        }
        static void onKeyRelease(uint8_t key){
            for (int i = 0; i<8; i++){

            }

        }
        static void setPortamento(uint8_t key){

        }
        static void noteOn(uint8_t key){
            tone(outputPin, goalFr);
            currentFr = goalFr;           
            states = noteOnHold;
            keyPlayed = key;                        
        };         
        static void stop(){
            noTone(outputPin);
            states = silent;
            
        }; 
        static void changePortamento(uint8_t input){};
        static void changePortamentoRes(uint8_t input){};
        static bool needToTick(unsigned long portaPreviousTime){
            if ((millis()-portaPreviousTime) >= portamentoTickTime){
                return true;
            }
            else {return false;}
        };
        static void changeChromaticPortamento(){};   
        static void setMIDIParams(){}; 
        static void changeBend(){};   
        static void tick(){};
};
MatrixKey keys[44];

//MonoPlayer pMonoPlayer;
// reads out the matrix and stores it to keysPressed[]
// also number of frequencyChart pressed
MonoPlayer monoPlayer;
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
// Rearranges keysPressed[] chronologically by comparing it to PKeysPressed. as its improbable that player will release one key and press other during one cycle of reading it only does it if number of pressed frequencyChart changes
/*void ReArrange()
{
    uint8_t i, j, k;
    uint8_t Array[8];
    for (i = 0; i < 8; i++)
    {
        Array[i] = 0;
    }
    k = 0;
    for (i = 0; i < pNumKeysPressed; i++)
    {
        for (j = 0; j < numKeysPressed; j++)
        {
            if (keysPressed[j] == PKeysPressed[i])
            {
                Array[k] = PKeysPressed[i];
                k++;
            }
        }
    }
    for (j = 0; j < numKeysPressed; j++)
    {
        bool Present = false;
        for (i = 0; i < pNumKeysPressed; i++)
        {
            if (keysPressed[j] == PKeysPressed[i])
            {
                Present = true;
            }
        }
        if (!Present)
        {
            Array[k] = keysPressed[j];
        }
    }

    keyToPlay = Array[numKeysPressed - 1];
    for (i = 0; i < 8; i++)
    {
        PKeysPressed[i] = Array[i];
    }
}*/
/*
void ChromPorta()
{
    if (abs(keyToPlay - keyPlayed) == 1)
    {
        Step = 0;
        tone(outputPin, frequencyChart[keyToPlay]);
        currentFr = frequencyChart[keyToPlay];
    }
    else
    {
        chrPortaKey = keyPlayed;
        portaRes = abs(portamento / (keyToPlay - keyPlayed));
        // only to assign a sign to Step
        Step = frequencyChart[keyToPlay] - frequencyChart[keyPlayed];
    }
}
// Plays the frequencyChart and sets up values for use in PortamentoMono()
void SetPlayMono()
{
    if (numKeysPressed > 0)
    {
        if (NotPlaying)
        {
            NotPlaying = false;
            keyPlayed = keyToPlay;
            tone(outputPin, frequencyChart[keyPlayed]);
            currentFr = frequencyChart[keyPlayed];
            Step = 0;
        }
        else if (numKeysPressed == 1)
        {
            if (keyToPlay != keyPlayed)
            {
                if (portamento != 0)
                {
                    portaStart = millis();
                    if (chrPorta)
                    {
                        ChromPorta();
                    }
                    else
                    {
                        Step = float(frequencyChart[keyToPlay] - frequencyChart[keyPlayed]) / portamento;
                    }
                }
                else
                {
                    Step = 0;
                    tone(outputPin, frequencyChart[keyToPlay]);
                    currentFr = frequencyChart[keyToPlay];
                }
                keyPlayed = keyToPlay;
            }
        }
        else if ((keyToPlay != keyPlayed) and (numKeysPressed > 1))
        {
            if (portamento != 0)
            {
                portaStart = millis();
                if (chrPorta)
                {
                    ChromPorta();
                }
                else
                {
                    Step = float(frequencyChart[keyToPlay] - frequencyChart[keyPlayed]) / portamento;
                }
            }
            else
            {
                Step = 0;
                tone(outputPin, frequencyChart[keyToPlay]);
                currentFr = frequencyChart[keyToPlay];
            }
            keyPlayed = keyToPlay;
        }
    }
    else if (!NotPlaying)
    {
        noTone(outputPin);
        NotPlaying = true;
        currentFr = 0;
        Step = 0;
    }
}
// executes frequency changes for portamento
void PortamentoMono()
{
    if (!NotPlaying)
    {
        int TimeDif = millis() - portaStart;
        if ((frequencyChart[keyPlayed] != currentFr) && (TimeDif >= portaRes))
        {
            if (Step > 0)
            {
                if (chrPorta)
                {
                    chrPortaKey++;
                    portaStart = millis();
                    currentFr = frequencyChart[chrPortaKey];
                    if (chrPortaKey == keyPlayed)
                    {
                        Step = 0;
                    }
                    tone(outputPin, int(currentFr));
                }
                else if (currentFr > frequencyChart[keyPlayed])
                {
                    currentFr = frequencyChart[keyPlayed];
                    tone(outputPin, int(currentFr));
                }
                else
                {
                    currentFr = currentFr + (Step * TimeDif);
                    portaStart = millis();
                    tone(outputPin, int(currentFr));
                    if (currentFr > frequencyChart[keyPlayed])
                    {
                        currentFr = frequencyChart[keyPlayed];
                        tone(outputPin, int(currentFr));
                    }
                }
            }
            else
            {
                if (chrPorta)
                {
                    chrPortaKey--;
                    portaStart = millis();
                    currentFr = frequencyChart[chrPortaKey];
                    if (chrPortaKey == keyPlayed)
                    {
                        Step = 0;
                    }
                    tone(outputPin, int(currentFr));
                }
                else if (currentFr < frequencyChart[keyPlayed])
                {
                    currentFr = frequencyChart[keyPlayed];
                    tone(outputPin, int(currentFr));
                }
                else
                {
                    currentFr = currentFr + (Step * TimeDif);
                    portaStart = millis();
                    tone(outputPin, int(currentFr));
                    if (currentFr < frequencyChart[keyPlayed])
                    {
                        currentFr = frequencyChart[keyPlayed];
                        tone(outputPin, int(currentFr));
                    }
                }
            }
        }
    }
}
*/
// assigning frequency values, may be better to use table but that was faster then searching for one
void setTheFreq()
{
    int i;
    for (i=29; i<sizeof(frequencyChart); i++){
        frequencyChart[i] =round(440.0 * pow(float(i), ((float(i)-69)/12)));
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

    for (uint8_t i = 5; i < 45; i++)
    {
        keys[i].onPress = &MonoPlayer::onKeyPress;
        keys[i].onRelease = &MonoPlayer::onKeyRelease;
    }
}
void loop()
{
    ReadLoop();   
    
}
