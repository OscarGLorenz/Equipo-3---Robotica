#ifdef __AVR_ATmega32U4__
#error ESTAS SUBIENDO EL MASTER AL SLAVE, ¡INUTIL!
#endif

#include <stdio.h>
#include "ENCODERINO.h"  // Clase Encoderino
#include <Wire.h>
#include <Servo.h>

#define ServoG A0

#define OLED_RESET 4
// Pines del puente H
#define PWMA 8
#define AIN1 9
#define AIN2 10
#define BIN1 11
#define BIN2 12
#define PWMB 13

// Pines del encoder
#define ENCODER_1 2
#define ENCODER_2 3
#define ENCODER_SW 4
long time = millis();

bool guiado = false;
int posicion = 0;
int anterior = 0;
unsigned long ultimaInterrupcion = 0;
void isr() {
  unsigned long tiempoInterrupcion = millis();

  if (tiempoInterrupcion - ultimaInterrupcion > 5) {

    if (digitalRead(ENCODER_2) == HIGH) {
      posicion --;
    } else {
      posicion ++;
    }

    ultimaInterrupcion = tiempoInterrupcion;
  }
}


Servo ServoGarra;
unsigned long clawUntilMillis = 0;  // Tiempo funcionamiento de la garra

// Referencia a serial y pin del endstop
Encoderino encoder1(&Serial1, A1);
Encoderino encoder2(&Serial2, A2);
Encoderino encoder3(&Serial3, A3);

// Array para indexar
Encoderino * encoders[] = {&encoder1, &encoder2, &encoder3};


void setup() {  
    Serial.begin(115200);

     // Serial.setTimeout(100);
  pinMode(AIN2, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  // Iniciar encoderinos
  encoders[0]->init();
  encoders[1]->init();
  encoders[2]->init();
  ServoGarra.attach(ServoG);

  attachInterrupt(digitalPinToInterrupt(ENCODER_1), isr, RISING); // Flanco bajada en el encoder

}

char command[1024];
void loop() {  
    // Actualiza encoders, revisa si han chocado con el endstop y si se ha preguntado la posición desde MATLAB
  encoders[0]->update();
  encoders[1]->update();
  encoders[2]->update();

 static unsigned long eachWrite = millis();
  if (millis() - eachWrite > 200) {
    char cmd[40] = {0};                  // Se manda la posición con formato %3.3f

    float q1 = encoders[0]->getPos(); // Obtención de la posición
    float q2 = encoders[1]->getPos(); // Obtención de la posición
    float q3 = encoders[2]->getPos(); // Obtención de la posición
    char aux1[10]; dtostrf(q1, 3, 3, aux1);
    char aux2[10]; dtostrf(q2, 3, 3, aux2);
    char aux3[10]; dtostrf(q3, 3, 3, aux3);

    strcat(cmd, aux1);
    strcat(cmd, " ");
    strcat(cmd, aux2);
    strcat(cmd, " ");
    strcat(cmd, aux3);
    strcat(cmd, " ");

    Serial.println(cmd);

    eachWrite = millis();
  }
  
  if (Serial.available()) {
     String cmd = Serial.readStringUntil('\n');
     cmd.toCharArray(command, 1024);


     // "command" now contains the full command
    //Serial.println(command);

    char jcode;
    int id;
    int motor;
    float value;
    
    char * ptr = strtok(command, " ");
    jcode = ptr[0];
    id = atoi(&ptr[1]);
      
    if (jcode == 'J') {                  // Si es un comando J...

      switch (id) {

        case 0:                          // Comando de Home
          encoders[0]->goHome();         // Manda a los motores orden de Home
          encoders[1]->goHome();
          encoders[2]->goHome();
          ServoGarra.write(90);
          guiado = false;
          break;


        case 1:                          // Comando de posicion
          ptr = strtok (NULL, " ");
          motor = atoi(&ptr[1]);
          ptr = strtok (NULL, " ");
          value = atof(ptr);
          
          if (motor == 4)
            ServoGarra.write(value);
          else
            encoders[motor - 1]->goPos(value); // Manda el motor correspondiente a la posición
          
          break;


        case 2:                          // Comando de velocidad
          ptr = strtok (NULL, " ");
          motor = atoi(&ptr[1]);
          ptr = strtok (NULL, " ");
          value = atof(ptr);
          
          encoders[motor - 1]->speed(value); // Manda el motor correspondiente a una velocidad
          
          break;


        case 3:                          // Comando de desactivar los stepper
          encoders[0]->disable();
          encoders[1]->disable();
          encoders[2]->disable();
          break;


        case 4:                          // Comando de herramienta
          ptr = strtok (NULL, " ");
          motor = atoi(&ptr[1]);
          ptr = strtok (NULL, " ");
          value = atof(ptr);

          if (motor == 1) { //Garra
            clawUntilMillis = fabs(value) + millis();
            if (value > 0) {
              analogWrite(PWMB, 255);
              digitalWrite(BIN1, HIGH);
              digitalWrite(BIN2, LOW);
            } else {
              analogWrite(PWMB, 255);
              digitalWrite(BIN1, LOW);
              digitalWrite(BIN2, HIGH);
            }
          } else if (motor == 2) {        //Bomba
            digitalWrite(AIN1, HIGH);
            digitalWrite(AIN2, LOW);
            analogWrite(PWMA, fabs(value));
          }
          break;

        case 5:                          // Interpolación con splines
          char buffer[1024];

          ptr = strtok(&command[3],"_");
          buffer[0] = '5'; buffer[1] = ' '; buffer[2] = '\0'; 
          strcat(buffer,ptr);
          encoders[0]->write(buffer);
        //  Serial.println(buffer);

          delay(100);
          ptr = strtok(NULL,"_");
          buffer[0] = '5'; buffer[1] = ' '; buffer[2] = '\0'; 
          strcat(buffer,ptr);
          encoders[1]->write(buffer);
          //Serial.println(buffer);

          delay(100);
          ptr = strtok(NULL,"_");
          buffer[0] = '5'; buffer[1] = ' '; buffer[2] = '\0'; 
          strcat(buffer,ptr);
          encoders[2]->write(buffer);
        //  Serial.println(buffer);


          break;
        case 6:
          anterior = posicion = encoders[0]->getPos();
          encoders[1]->disable();
          encoders[2]->disable();
          guiado = !guiado;
          break;
      }
    }

  }

  if (guiado && (millis() - time > 500) && posicion != anterior) { 
    anterior = posicion = constrain(posicion, 0, 250); 
    encoders[0]->goPos(anterior); 
    time = millis(); 
  } 
}
