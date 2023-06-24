#include <Arduino.h>

class Schrittmotor{
    public:
        Schrittmotor(int dirPin, int stepPin, int enablePin){
          this->dirPin = dirPin;
          this->stepPin = stepPin;
          this->enablePin = enablePin;

          this->currentStepPin = false;

          pinMode(dirPin,OUTPUT);
          pinMode(stepPin,OUTPUT);
          pinMode(enablePin,OUTPUT);
        }

        void Enable(bool On){
          digitalWrite(enablePin, On);
        }

        void Forward(){
          digitalWrite(dirPin, LOW);
          doStep();
        }
        void Backward(){
          digitalWrite(dirPin, HIGH);
          doStep();
        }

    private:
        int dirPin;
        int stepPin;
        int enablePin;

        bool currentStepPin;

        void doStep(){
            digitalWrite(stepPin, currentStepPin);
            currentStepPin = !currentStepPin;
        }
};