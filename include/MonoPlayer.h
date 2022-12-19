#include <Arduino.h>

class MonoPlayer
{
private:
    float currentFr;
    uint16_t goalFr;
    uint8_t keyPlayed;
    float step;
    unsigned long portamentoPrevTime;
    enum
    {
        silent,
        noteOnHold,
        inPortameno,
        inChrPortamento
    } states;
    uint8_t portamento;
    uint8_t portamentoTickTime;
    bool chromaticPortamento;
    bool playFromMIDI;
    bool sendMIDI;
    uint8_t MIDIchannel;
    uint8_t keysPressed[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    float bend;
    uint16_t frequencyCalc(uint8_t key){
        return int(440.0 * pow(2.0, ((double(key) - 69) / 12)));
    }

public:
    MonoPlayer()
    {
        step = 0.0;
        playFromMIDI = false;
        sendMIDI = false;
        portamento = 0;
        portamentoTickTime = 1;
        chromaticPortamento = false;

        states = silent;
        bend = 1.0;
    };
    void onNewKey(uint8_t key)
    {
        goalFr = frequencyCalc(key) * bend;
        switch (states)
        {
        case silent:
            noteOn(key);
            break;
        default:
            if (portamento == 0)
            {
                noteOn(key);
                break;
            }
            else{
               if (chromaticPortamento){
                    setChromaticPortamento(key);
                }
                else {
                    setPortamento(key);
                } 
            }

            break;
        }
    }
    void onKeyPress(uint8_t key)
    {
        addToArray(key);
        onNewKey(key);
    }
    void addToArray(uint8_t key)
    {

        for (uint8_t i = 0; i < 8; i++)
        {
            if (keysPressed[i] == 0)
            {
                keysPressed[i] = key;
                break;
            }
        }
    }

    void deleteFromArray(uint8_t key)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            if (keysPressed[i] == key)
            {
                keysPressed[i] = 0;
                for (uint8_t j = i; i < 7; i++)
                {
                    if (keysPressed[j] == 0)
                    {
                        if (keysPressed[j + 1] != 0)
                        {
                            keysPressed[j] = keysPressed[i + 1];
                            keysPressed[j + 1] = 0;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                break;
            }
        }
    }

    void onKeyRelease(uint8_t key)
    {
        deleteFromArray(key);
        if (keysPressed[0] == 0)
        {
            stop();
        }
        else
        {
            for (int i = 7; i >= 0; i--)
            {
                if (keysPressed[i] != 0)
                {
                    onNewKey(keysPressed[i]);
                    break;
                }
            }
        }
    }
    void setPortamento(uint8_t key)
    {
        
    }
    void setChromaticPortamento(uint8_t key)
    {

    }
    void noteOn(uint8_t key)
    {
        tone(outputPin, goalFr);
        currentFr = goalFr;
        states = noteOnHold;
        keyPlayed = key;
    };
    void stop()
    {
        noTone(outputPin);
        states = silent;
    };
    void tick()
    {
        if (needToTick(portamentoPrevTime)){
        switch (states)
        {
        case inChrPortamento:
            
            break;
        case inPortameno:
            
            break;
        default:
            break;
        }
        }
    }
    bool needToTick(unsigned long portaPreviousTime)
    {
        if ((millis() - portaPreviousTime) >= portamentoTickTime)
        {
            return true;
        }
        else
        {
            return false;
        }
    };

};
