#include <Arduino.h>


class Button{

typedef void (*parameterizedCallbackFunction)(uint8_t);
private:
    bool wasOn;
    uint8_t pin;
    parameterizedCallbackFunction _onPress;
    parameterizedCallbackFunction _onRelease;
    uint8_t _onPressParam;
    uint8_t _onReleaseParam;
public:
    Button(){
        _onPress = NULL;
        _onRelease = NULL;  
        _onPressParam = 0;
        _onReleaseParam = 0;
    }
    void assignPin(uint8_t _pin){
        pin = _pin;
    }
    void attachOnPress(parameterizedCallbackFunction func, uint8_t param){
        _onPress = func;
        _onPressParam = param;
    }
    void attachOnRelease(parameterizedCallbackFunction func, uint8_t param){
        _onRelease = func;
        _onReleaseParam = param;
    }
    void tick(){
        if (digitalRead(pin) == LOW){
            if (!wasOn){
                wasOn = true;
                if (_onPress) _onPress(_onPressParam);
                return;
            }            
            return;
        }
        else if (wasOn){
            wasOn = false;
            if (_onRelease) _onRelease(_onReleaseParam);
        }

    }

};