#include <Arduino.h>
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

// Setting the pins for scanning matrix
uint8_t OutPins[] = {8, 7, 6, 5, 4, 3, 2};
uint8_t InPins[] = {9, 10, 11, 12, 14, 15, 16, 17};
const int OutPin = 13;
// Array Keys stores frequency values for keys. values are calculetd during setup;
//  A 440 Hz is [45]
int Keys[83];

// Octave shift; range 0 -3
uint8_t OctaveShift = 1;

// ChrPortaKey used for chromatic portamento
uint8_t KeyPlayed, KeyToPlay, NumKeysPressed, PNumKeysPressed, ChrPortaKey;

int GoalFr;
// we use int in tone(), but calculate in float to minimize error
float CurrentFr;

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
                    KeysPressed[NumKeysPressed] = ButtonNo + (12 * OctaveShift);
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
            CurrentFr = Keys[KeyPlayed];
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
                        if (abs(KeyToPlay - KeyPlayed) == 1)
                        {
                            Step = 0;
                            tone(OutPin, Keys[KeyToPlay]);
                            CurrentFr = Keys[KeyToPlay];
                        }
                        else
                        {
                            ChrPortaKey = KeyPlayed;
                            PortaRes = abs(Portamento / (KeyToPlay - KeyPlayed));
                            // only to assign a sign to Step
                            Step = Keys[KeyToPlay] - Keys[KeyPlayed];
                        }
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
                    CurrentFr = Keys[KeyToPlay];
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
                    if (abs(KeyToPlay - KeyPlayed) == 1)
                    {
                        Step = 0;
                        tone(OutPin, Keys[KeyToPlay]);
                        CurrentFr = Keys[KeyToPlay];
                    }
                    else
                    {
                        ChrPortaKey = KeyPlayed;
                        PortaRes = abs(Portamento / (KeyToPlay - KeyPlayed));
                        // only to assign a sign to Step
                        Step = Keys[KeyToPlay] - Keys[KeyPlayed];
                    }
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
                CurrentFr = Keys[KeyToPlay];
            }
            KeyPlayed = KeyToPlay;
        }
    }
    else if (!NotPlaying)
    {
        noTone(OutPin);
        NotPlaying = true;
        CurrentFr = 0;
        Step = 0;
    }
}
// executes frequency changes for portamento
void PortamentoMono()
{
    if (!NotPlaying)
    {
        int TimeDif = millis() - PortaStart;
        if ((Keys[KeyPlayed] != CurrentFr) && (TimeDif >= PortaRes))
        {
            if (Step > 0)
            {
                if (ChrPorta)
                {
                    ChrPortaKey++;
                    PortaStart = millis();
                    CurrentFr = Keys[ChrPortaKey];
                    if (ChrPortaKey == KeyPlayed)
                    {
                        Step = 0;
                    }
                    tone(OutPin, int(CurrentFr));
                }
                else if (CurrentFr > Keys[KeyPlayed])
                {
                    CurrentFr = Keys[KeyPlayed];
                    tone(OutPin, int(CurrentFr));
                }
                else
                {
                    CurrentFr = CurrentFr + (Step * TimeDif);
                    PortaStart = millis();
                    tone(OutPin, int(CurrentFr));
                    if (CurrentFr > Keys[KeyPlayed])
                    {
                        CurrentFr = Keys[KeyPlayed];
                        tone(OutPin, int(CurrentFr));
                    }
                }
            }
            else
            {
                if (ChrPorta)
                {
                    ChrPortaKey--;
                    PortaStart = millis();
                    CurrentFr = Keys[ChrPortaKey];
                    if (ChrPortaKey == KeyPlayed)
                    {
                        Step = 0;
                    }
                    tone(OutPin, int(CurrentFr));
                }
                else if (CurrentFr < Keys[KeyPlayed])
                {
                    CurrentFr = Keys[KeyPlayed];
                    tone(OutPin, int(CurrentFr));
                }
                else
                {
                    CurrentFr = CurrentFr + (Step * TimeDif);
                    PortaStart = millis();
                    tone(OutPin, int(CurrentFr));
                    if (CurrentFr < Keys[KeyPlayed])
                    {
                        CurrentFr = Keys[KeyPlayed];
                        tone(OutPin, int(CurrentFr));
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
    Keys[5] = 44;
    Keys[6] = 46;
    Keys[7] = 49;
    Keys[8] = 52;
    Keys[9] = 55;
    for (i = 0; i < 11; i++)
    {
        Keys[i + 10] = int(Keys[i + 9] * 1.05946);
    }
    Keys[21] = 110;
    for (i = 0; i < 11; i++)
    {
        Keys[i + 22] = int(Keys[i + 21] * 1.05946);
    }
    Keys[33] = 220;
    for (i = 0; i < 11; i++)
    {
        Keys[i + 34] = int(Keys[i + 33] * 1.05946);
    }
    Keys[45] = 440;
    for (i = 0; i < 11; i++)
    {
        Keys[i + 46] = int(Keys[i + 45] * 1.05946);
    }
    Keys[57] = 880;
    for (i = 0; i < 11; i++)
    {
        Keys[i + 58] = int(Keys[i + 57] * 1.05946);
    }
    Keys[69] = 1760;
    for (i = 0; i < 11; i++)
    {
        Keys[i + 70] = int(Keys[i + 69] * 1.05946);
    }
    Keys[81] = 3520;
    for (i = 0; i < 11; i++)
    {
        Keys[i + 82] = int(Keys[i + 81] * 1.05946);
    }
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