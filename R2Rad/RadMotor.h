#include <Arduino.h>

class RadMotor{
    public:
        RadMotor(int dirPin, int enablePin){
            this->dirPin = dirPin;
            this->enablePin = enablePin;
            
            pinMode(dirPin,OUTPUT);
            pinMode(enablePin,OUTPUT);
        }

        void Stop(){
            digitalWrite(dirPin, LOW);
            digitalWrite(enablePin, LOW);
        }
        void Forward(uint8_t speed){
            digitalWrite(dirPin, LOW);
            analogWrite(enablePin, speed);
        }
        void Backward(uint8_t speed){
            digitalWrite(dirPin, HIGH);
            analogWrite(enablePin, speed);
        }

    private:
        int dirPin;
        int enablePin;
};