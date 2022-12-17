#include <Arduino.h>
#include <Controlino.h>
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
uint16_t Keys[79];

// Setting the pins for scanning matrix
uint8_t OutPins[] = {8, 7, 6, 5, 4, 3, 2};
uint8_t InPins[] = {9, 10, 11, 12, 14, 15, 16, 17};
const int OutPin = 13;
// Array Keys stores frequency values for keys. values are calculetd during setup;
//  A 440 Hz is [45]


// Octave shift; range 0 -3
uint8_t OctaveShift = 1;

// ChrPortaKey used for chromatic portamento
uint8_t KeyPlayed, KeyToPlay, NumKeysPressed, PNumKeysPressed, ChrPortaKey;

int GoalFr;


// Arrays that store all keys pressed in current and previous reading, empty array elements are zeros. PKeysPressed has keys sorted in it chronologically, with most recent key having greater index
int KeysPressed[8], PKeysPressed[8];

// Portamento to be set as analog in: for now its const. 0 for no portamento
int Portamento = 0;
// PortaRes is resolution in millis for arpeggiated effects; set to 1 for no effect. also shoul be read from analog input
int PortaRes = 20;

// set to true to go through a chromatic and false to go linearly through frequencies; will immideately play a half-tone without delay
bool ChrPorta = true;

// Step is calculated for linear portamento
float Step;

unsigned long PortaStart;

unsigned long CurrentTime;

//this class plays without portamento
class monoPlayer{
    protected: 
        static  float currentFr;
        bool isSilent;
    public:
        monoPlayer(){
            isSilent = true;
            playFromMIDI = false;
            sendMIDI = false;
        };
        bool playFromMIDI;
        bool sendMIDI;        
        uint8_t MIDIchannel;
        virtual void playTheKey(uint8_t key, float bend){
            currentFr = (Keys[KeyToPlay])*bend;
            isSilent = false;
            tone(OutPin, int(currentFr));
        }; 
        //should be able to send here MIDI note as well in the future
        void stop(){
            noTone(OutPin);
        };        
};


// reads out the matrix and stores it to KeysPressed[]
// also number of keys pressed

void ReadLoop()
{
    uint8_t i, j;
    PNumKeysPressed = NumKeysPressed;
    NumKeysPressed = 0;
    for (i = 0; i < 7; i++)
    {
        digitalWrite(OutPins[i], LOW);
        for (j = 0; j < 8; j++)
        {
            if (digitalRead(InPins[j]) == LOW)
            {
                // buttons not used in my exact keyboard are 0-4; and 49-55
                // thats the place you can see if it was pressed. but you should store it in some way, e.g. in two arrays as i did here with keys
                int ButtonNo = (i * 8) + j;

                if (NumKeysPressed < 8)
                {
                    KeysPressed[NumKeysPressed] = 24 + ButtonNo + (12 * OctaveShift); // +24 to make it as im MIDI so [0;5]  (leftmost key)  is F1 (MIDI 29)
                }
                NumKeysPressed++;
            }
        }
        digitalWrite(OutPins[i], HIGH);
    }
}
// Rearranges KeysPressed[] chronologically by comparing it to PKeysPressed. as its improbable that player will release one key and press other during one cycle of reading it only does it if number of pressed keys changes
void ReArrange()
{
    uint8_t i, j, k;
    uint8_t Array[8];
    for (i = 0; i < 8; i++)
    {
        Array[i] = 0;
    }
    k = 0;
    for (i = 0; i < PNumKeysPressed; i++)
    {
        for (j = 0; j < NumKeysPressed; j++)
        {
            if (KeysPressed[j] == PKeysPressed[i])
            {
                Array[k] = PKeysPressed[i];
                k++;
            }
        }
    }
    for (j = 0; j < NumKeysPressed; j++)
    {
        bool Present = false;
        for (i = 0; i < PNumKeysPressed; i++)
        {
            if (KeysPressed[j] == PKeysPressed[i])
            {
                Present = true;
            }
        }
        if (!Present)
        {
            Array[k] = KeysPressed[j];
        }
    }

    KeyToPlay = Array[NumKeysPressed - 1];
    for (i = 0; i < 8; i++)
    {
        PKeysPressed[i] = Array[i];
    }
}
void ChromPorta()
{
    if (abs(KeyToPlay - KeyPlayed) == 1)
    {
        Step = 0;
        tone(OutPin, Keys[KeyToPlay]);
        currentFr = Keys[KeyToPlay];
    }
    else
    {
        ChrPortaKey = KeyPlayed;
        PortaRes = abs(Portamento / (KeyToPlay - KeyPlayed));
        // only to assign a sign to Step
        Step = Keys[KeyToPlay] - Keys[KeyPlayed];
    }
}
// Plays the keys and sets up values for use in PortamentoMono()
void SetPlayMono()
{
    if (NumKeysPressed > 0)
    {
        if (NotPlaying)
        {
            NotPlaying = false;
            KeyPlayed = KeyToPlay;
            tone(OutPin, Keys[KeyPlayed]);
            currentFr = Keys[KeyPlayed];
            Step = 0;
        }
        else if (NumKeysPressed == 1)
        {
            if (KeyToPlay != KeyPlayed)
            {
                if (Portamento != 0)
                {
                    PortaStart = millis();
                    if (ChrPorta)
                    {
                        ChromPorta();
                    }
                    else
                    {
                        Step = float(Keys[KeyToPlay] - Keys[KeyPlayed]) / Portamento;
                    }
                }
                else
                {
                    Step = 0;
                    tone(OutPin, Keys[KeyToPlay]);
                    currentFr = Keys[KeyToPlay];
                }
                KeyPlayed = KeyToPlay;
            }
        }
        else if ((KeyToPlay != KeyPlayed) and (NumKeysPressed > 1))
        {
            if (Portamento != 0)
            {
                PortaStart = millis();
                if (ChrPorta)
                {
                    ChromPorta();
                }
                else
                {
                    Step = float(Keys[KeyToPlay] - Keys[KeyPlayed]) / Portamento;
                }
            }
            else
            {
                Step = 0;
                tone(OutPin, Keys[KeyToPlay]);
                currentFr = Keys[KeyToPlay];
            }
            KeyPlayed = KeyToPlay;
        }
    }
    else if (!NotPlaying)
    {
        noTone(OutPin);
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
        int TimeDif = millis() - PortaStart;
        if ((Keys[KeyPlayed] != currentFr) && (TimeDif >= PortaRes))
        {
            if (Step > 0)
            {
                if (ChrPorta)
                {
                    ChrPortaKey++;
                    PortaStart = millis();
                    currentFr = Keys[ChrPortaKey];
                    if (ChrPortaKey == KeyPlayed)
                    {
                        Step = 0;
                    }
                    tone(OutPin, int(currentFr));
                }
                else if (currentFr > Keys[KeyPlayed])
                {
                    currentFr = Keys[KeyPlayed];
                    tone(OutPin, int(currentFr));
                }
                else
                {
                    currentFr = currentFr + (Step * TimeDif);
                    PortaStart = millis();
                    tone(OutPin, int(currentFr));
                    if (currentFr > Keys[KeyPlayed])
                    {
                        currentFr = Keys[KeyPlayed];
                        tone(OutPin, int(currentFr));
                    }
                }
            }
            else
            {
                if (ChrPorta)
                {
                    ChrPortaKey--;
                    PortaStart = millis();
                    currentFr = Keys[ChrPortaKey];
                    if (ChrPortaKey == KeyPlayed)
                    {
                        Step = 0;
                    }
                    tone(OutPin, int(currentFr));
                }
                else if (currentFr < Keys[KeyPlayed])
                {
                    currentFr = Keys[KeyPlayed];
                    tone(OutPin, int(currentFr));
                }
                else
                {
                    currentFr = currentFr + (Step * TimeDif);
                    PortaStart = millis();
                    tone(OutPin, int(currentFr));
                    if (currentFr < Keys[KeyPlayed])
                    {
                        currentFr = Keys[KeyPlayed];
                        tone(OutPin, int(currentFr));
                    }
                }
            }
        }
    }
}
// assigning frequency values, may be better to use table but that was faster then searching for one
void SetTheFreq()
{
    int i;
    for (i=29; i<sizeof(Keys); i++){
        Keys[i] =round(440.0 * pow(float(i), ((float(i)-69)/12)));
        Serial.println(Keys[i]);
    };    
}

void setup()
{
    Serial.begin(9600);
    pinMode(OutPin, OUTPUT);
    digitalWrite(OutPin, LOW);
    uint8_t i, j;
    // initialize matrix for scanning
    for (i = 0; i < 7; i++)
    {
        pinMode(OutPins[i], OUTPUT);
        digitalWrite(OutPins[i], HIGH);
    }
    for (i = 0; i < 8; i++)
    {
        pinMode(InPins[i], INPUT_PULLUP);
    }

    NotPlaying = true;
    NumKeysPressed = 0;

    pKeys = Keys;

    // calculate values for Keys[]
    SetTheFreq();

    for (i = 0; i < 8; i++)
    {
        KeysPressed[i] = 0;
    }
}
void loop()
{
    ReadLoop();
    // any changes in state are only result of change ofnumber of keys pressed
    if (NumKeysPressed != PNumKeysPressed)
    {
        ReArrange();
        SetPlayMono();
    }
    // only to change frequencies during portamento
    if ((Portamento != 0) or (Step != 0))
    {
        PortamentoMono();
    }
}