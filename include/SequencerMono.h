#include <Arduino.h>

class SequencerMono{
private:
    enum mode {seqRec, seqPlay, realTimeRec, RealTimePlay};
    int16_t sequenseLength_ms;
    int8_t sequenceRes_ms;
    int8_t bpm;
public:
    SequencerMono(mode state){
        sequenceRes_ms = 1;
    }
    void tick(){

    }
};