#include <Arduino.h>

class RadMotor{
    public:
        RadMotor(int dirPin, int enablePin, int signalPin){
            this->dirPin = dirPin;
            this->enablePin = enablePin;
            this->signalPin = signalPin;
            
            pinMode(dirPin,OUTPUT);
            pinMode(enablePin,OUTPUT);
            pinMode(signalPin,INPUT);
        }
        
        void update(){
          if(steps == 0){
            Stop();
          }else{
            bool read = digitalRead(signalPin);
            if(read != lastStep){
              steps--;
              lastStep = read;
            }
            if(steps == 0)
              Stop();
          }
        }

        bool isDriving(){
          return steps > 0;
        }

        void Stop(){
            digitalWrite(dirPin, LOW);
            digitalWrite(enablePin, LOW);
            steps = 0;
        }
        void Forward(uint8_t speed, uint32_t steps){
            digitalWrite(dirPin, LOW);
            analogWrite(enablePin, speed);
            this->steps = steps;
        }
        void Backward(uint8_t speed, uint32_t steps){
            digitalWrite(dirPin, HIGH);
            analogWrite(enablePin, speed);
            this->steps = steps;
        }

    private:
        int dirPin;
        int enablePin;
        int signalPin;
        bool lastStep;
        uint32_t steps;
};