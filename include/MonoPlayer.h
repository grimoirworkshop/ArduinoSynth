#include <Arduino.h>

class MonoPlayer
{
private:
    float currentFr;
    uint16_t goalFr;
    uint8_t keyPlayed;
    uint8_t currentKey; // thats for use in chromatic portamento
    float step;
    unsigned long portamentoPrevTime;
    enum
    {
        silent,
        noteOnHold,
        inPortameno,
        inChrPortamento
    } states;
    uint16_t portamento;
    uint16_t portamentoTickTime;
    bool chromaticPortamento;
    bool playFromMIDI;
    bool sendMIDI;
    uint8_t MIDIchannel;
    uint8_t keysPressed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    uint16_t frequencyCalc(uint8_t key)
    {
        return int(440.0 * pow(2.0, ((double(key) - 69) / 12)));
    }

public:
    MonoPlayer()
    {
        step = 0.0;
        playFromMIDI = false;
        sendMIDI = false;
        portamento = 0;
        portamentoTickTime = 50;
        chromaticPortamento = false;

        states = silent;
    };

    void onNewKey(uint8_t key, float bend)
    {
        goalFr = frequencyCalc(key);
        switch (states)
        {
        case silent:
            noteOn(key, bend);
            break;
        default:

            if (portamento == 0)
            {
                noteOn(key, bend);
                break;
            }
            if (chromaticPortamento)
            {
                setChromaticPortamento(key);
                break;
            }
            else
            {
                setPortamento(key);
                break;
            }
        }
    }
    void onKeyPress(uint8_t key)
    {
        addToArray(key);
        onNewKey(key, 1.0);
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
                    onNewKey(keysPressed[i], 1.0);
                    break;
                }
            }
        }
    }
    void setPortamento(uint8_t key)
    {
        step = (goalFr - currentFr) / portamento;
        portamentoPrevTime = millis();
        states = inPortameno;
        keyPlayed = key;
    }
    void setChromaticPortamento(uint8_t key)
    {
        if ((key+1  == keyPlayed) or ((key-1  == keyPlayed))){
            noteOn(key, 1.0);
            return;
        }
        states = inChrPortamento;      
       
        portamentoTickTime = portamento/(abs(key - keyPlayed));
        Serial.println(portamentoTickTime);     

        currentKey = keyPlayed;
        keyPlayed = key;
        portamentoPrevTime = millis();
    }
    void noteOn(uint8_t key, float bend)
    {
        goalFr *= bend;
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
    void portamentoTick()
    {
        int timePassed = millis() - portamentoPrevTime;
        if (needToTick(timePassed))
        {
            portamentoPrevTime = millis();
            switch (states)
            {
            case inChrPortamento:
                currentKey = ((currentKey > keyPlayed) ? (currentKey - 1) : (currentKey + 1));
                if (currentKey == keyPlayed)
                {
                    states = noteOnHold;
                }
                currentFr = frequencyCalc(currentKey);
                tone(outputPin, currentFr);

                break;
            case inPortameno:
                currentFr += step * timePassed;
                if ((goalFr - currentFr) / step > 1)
                {
                    tone(outputPin, int(currentFr));

                    break;
                }
                states = noteOnHold;
                currentFr = goalFr;
                tone(outputPin, int(currentFr));
                break;
            default:
                break;
            }
        }
    }
    bool needToTick(uint16_t passedTime)
    {
        return (passedTime >= portamentoTickTime);
    };
    uint8_t onSequenserWrite(uint8_t i){
        return keysPressed[i];
    }
    void onBendChange(float bend)
    {
        if (states == noteOnHold)
        {
            noteOn(keyPlayed, bend);
        }
    }
};
