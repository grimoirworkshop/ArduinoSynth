#include <Arduino.h>
#include <OneButton.h>
#include <math.h>
//  thats basic functionality for monophonyc keyboard
//  it supports portamento as chromatic and linear with variable resolution to avchieve arpeggiated effects
//  it supports octave shifts from default you can move one oct down or two up
//  outputs as square wave to digital pin
//  yet to implement:
//  pitch bend
//  cv out
//  record and play sequences, ideally to external clock, should probably do as interrupt

// for prpgramm to know if its starting new sound or shifting from note to note
bool NotPlaying;
uint16_t keys[79];

// Setting the pins for scanning matrix
uint8_t outPins[] = {8, 7, 6, 5, 4, 3, 2};
uint8_t inPins[] = {9, 10, 11, 12, 14, 15, 16, 17};
const int outputPin = 13;
// Array keys stores frequency values for keys. values are calculetd during setup;
//  A 440 Hz is [45]


// Octave shift; range 0 -3
uint8_t octaveShift = 1;
OneButton buttons[11];
// chrPortaKey used for chromatic portamento
uint8_t  keyToPlay, numKeysPressed, pNumKeysPressed, chrPortaKey;

int GoalFr;


// Arrays that store all keys pressed in current and previous reading, empty array elements are zeros. PKeysPressed has keys sorted in it chronologically, with most recent key having greater index
int keysPressed[8], PKeysPressed[8];

// portamento to be set as analog in: for now its const. 0 for no portamento
int portamentoMillis = 0;
// portaRes is resolution in millis for arpeggiated effects; set to 1 for no effect. also shoul be read from analog input
int portaRes = 20;

// set to true to go through a chromatic and false to go linearly through frequencies; will immideately play a half-tone without delay
bool chrPorta = true;





unsigned long currentTime;

//this class plays without portamento
class MonoPlayer{
    private: 
        static  float currentFr;
        bool isSilent;
        uint16_t goalFr;
        uint8_t keyPlayed;
        float step;
        unsigned long portamentoStartTime;
        bool needToTick(unsigned long portaStart){
            
        }
    public:
        
        uint8_t portamento;
        int8_t    portamentoTickTime;
        bool chromaticPortamento;
        bool playFromMIDI;
        bool sendMIDI;  
        bool isPerformingPortamento; 
        uint8_t MIDIchannel;
        MonoPlayer(){
            step = 0.0;
            isSilent = true;
            playFromMIDI = false;
            sendMIDI = false;
            isPerformingPortamento = false;
            portamento = portamentoMillis;
            portamentoTickTime = portaRes;
            chromaticPortamento = chrPorta;
        }; 
        
        
        virtual void playTheKey(uint8_t key, float bend){
            goalFr = (keys[keyToPlay])*bend;            
            if (isSilent){
                currentFr = goalFr;
                isSilent = false;
                tone(outputPin, int(currentFr));
                keyPlayed = keyToPlay;
                isPerformingPortamento = false;
                portamentoStartTime = millis();
            }
            else if (!isPerformingPortamento && (currentFr != goalFr)){
                currentFr = goalFr;
                tone(outputPin, int(currentFr));
            }
            else if (portamento != 0){
                if (chromaticPortamento) {
                    if (keyPlayed != keyToPlay){
                        portamentoTickTime = portamento/(keyToPlay-keyPlayed);
                        keyPlayed = keyToPlay;
                    }
                    else{

                    }

                }
            }
            return;
        }; 
        //should be able to send here MIDI note as well in the future
        void stop(){
            noTone(outputPin);
            isSilent = true;
            return;
        };        
};


//MonoPlayer pMonoPlayer;
// reads out the matrix and stores it to keysPressed[]
// also number of keys pressed
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
            if (digitalRead(inPins[j]) == LOW)
            {
                // buttons not used in my exact keyboard are 0-4; and 49-55
                // thats the place you can see if it was pressed. but you should store it in some way, e.g. in two arrays as i did here with keys
                int buttonNo = (i * 8) + j;
                if (buttonNo < 5){
                    buttons[buttonNo].tick();
                }
                else if (buttonNo >48){
                    buttons[buttonNo].tick();
                }
                else if (numKeysPressed < 8)
                {
                    keysPressed[numKeysPressed] = 24 + buttonNo + (12 * octaveShift); // +24 to make it as im MIDI so [0;5]  (leftmost key)  is F1 (MIDI 29)
                }
                numKeysPressed++;
            }
        }
        digitalWrite(outPins[i], HIGH);
    }
}
// Rearranges keysPressed[] chronologically by comparing it to PKeysPressed. as its improbable that player will release one key and press other during one cycle of reading it only does it if number of pressed keys changes
void ReArrange()
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
}/*
void ChromPorta()
{
    if (abs(keyToPlay - keyPlayed) == 1)
    {
        Step = 0;
        tone(outputPin, keys[keyToPlay]);
        currentFr = keys[keyToPlay];
    }
    else
    {
        chrPortaKey = keyPlayed;
        portaRes = abs(portamento / (keyToPlay - keyPlayed));
        // only to assign a sign to Step
        Step = keys[keyToPlay] - keys[keyPlayed];
    }
}
// Plays the keys and sets up values for use in PortamentoMono()
void SetPlayMono()
{
    if (numKeysPressed > 0)
    {
        if (NotPlaying)
        {
            NotPlaying = false;
            keyPlayed = keyToPlay;
            tone(outputPin, keys[keyPlayed]);
            currentFr = keys[keyPlayed];
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
                        Step = float(keys[keyToPlay] - keys[keyPlayed]) / portamento;
                    }
                }
                else
                {
                    Step = 0;
                    tone(outputPin, keys[keyToPlay]);
                    currentFr = keys[keyToPlay];
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
                    Step = float(keys[keyToPlay] - keys[keyPlayed]) / portamento;
                }
            }
            else
            {
                Step = 0;
                tone(outputPin, keys[keyToPlay]);
                currentFr = keys[keyToPlay];
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
        if ((keys[keyPlayed] != currentFr) && (TimeDif >= portaRes))
        {
            if (Step > 0)
            {
                if (chrPorta)
                {
                    chrPortaKey++;
                    portaStart = millis();
                    currentFr = keys[chrPortaKey];
                    if (chrPortaKey == keyPlayed)
                    {
                        Step = 0;
                    }
                    tone(outputPin, int(currentFr));
                }
                else if (currentFr > keys[keyPlayed])
                {
                    currentFr = keys[keyPlayed];
                    tone(outputPin, int(currentFr));
                }
                else
                {
                    currentFr = currentFr + (Step * TimeDif);
                    portaStart = millis();
                    tone(outputPin, int(currentFr));
                    if (currentFr > keys[keyPlayed])
                    {
                        currentFr = keys[keyPlayed];
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
                    currentFr = keys[chrPortaKey];
                    if (chrPortaKey == keyPlayed)
                    {
                        Step = 0;
                    }
                    tone(outputPin, int(currentFr));
                }
                else if (currentFr < keys[keyPlayed])
                {
                    currentFr = keys[keyPlayed];
                    tone(outputPin, int(currentFr));
                }
                else
                {
                    currentFr = currentFr + (Step * TimeDif);
                    portaStart = millis();
                    tone(outputPin, int(currentFr));
                    if (currentFr < keys[keyPlayed])
                    {
                        currentFr = keys[keyPlayed];
                        tone(outputPin, int(currentFr));
                    }
                }
            }
        }
    }
}
*/
// assigning frequency values, may be better to use table but that was faster then searching for one
void SetTheFreq()
{
    int i;
    for (i=29; i<sizeof(keys); i++){
        keys[i] =round(440.0 * pow(float(i), ((float(i)-69)/12)));
        Serial.println(keys[i]);
    };    
}

void setup()
{
    Serial.begin(9600);
    pinMode(outputPin, OUTPUT);
    digitalWrite(outputPin, LOW);
    uint8_t i, j;
    
    // initialize matrix for scanning
    for (i = 0; i < 7; i++)
    {
        pinMode(outPins[i], OUTPUT);
        digitalWrite(outPins[i], HIGH);
    }
    for (i = 0; i < 8; i++)
    {
        pinMode(inPins[i], INPUT_PULLUP);
    }

    NotPlaying = true;
    numKeysPressed = 0;

    

    // calculate values for keys[]
    SetTheFreq();

    for (i = 0; i < 8; i++)
    {
        keysPressed[i] = 0;
    }
}
void loop()
{
    ReadLoop();
    // any changes in state are only result of change ofnumber of keys pressed
    if (numKeysPressed != pNumKeysPressed)
    {
        if (numKeysPressed != 0){
            ReArrange();
            monoPlayer.playTheKey(keyToPlay, 0.0);
        }
        else { 
            monoPlayer.stop();
        }
    }
    // only to change frequencies during portamento
    
}