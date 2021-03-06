#include <i2cmaster.h>
#include <Servo.h> 

#define T_AMB 0x06
#define T_OBJ 0x07
#define WAIT_TIME 15 // number of seconds between each measure
#define N_SAMPLES 3 // number of samples to average between two measures
#define DEBUG false
#define TEST false
#define N_STOPS 3
int stop_positions[N_STOPS] = {0, 70, 150};

float temp_amb[N_SAMPLES];
float temp_obj[N_SAMPLES];

Servo myservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 

int current_pos = 0;  // variable to store the servo position 
int old_pos = 0;
tr
char *float2s(float f, unsigned int digits=2) {
  static char buf[10];
  return dtostrf(f, 7, digits, buf);
}

void send_results(int angle, float amb, float obj) {
    // Protocol
    //
    // Each message is coded on 17 bytes
    // $ PPP,+AAA.AA,+OOO.OO<CR>
    // Where:
    //  PPP is the servo angle
    //  +AAA.AA is the signed ambient temperature
    //  +OOO.OO is the signed object temperature
    //
    // Ex: $090-010.15+030.42\r
    char buffer[22];
    sprintf(buffer, "@%03d,", angle);
    strncpy(buffer+5, float2s(amb-273.15), 7);
    strncpy(buffer+12, ",", 1);
    strncpy(buffer+13, float2s(obj-273.15), 7);
    buffer[20] = '!';
    buffer[21] = '\0';
    Serial.println(buffer);
}

void reset_measures(float *arr) {
    for (int i = 0 ; i < sizeof(arr) ; i++) {
        arr[i] = -100.0; // we know the MLX sensor's range does not go below -40 °C
    }
}

float average(float *arr) {
    int n = 0;
    float aux = 0.0;
    for (int i = 0 ; i < sizeof(arr) ; i++) {
        if (arr[i] >= -40.0) { // the temperature is above the minimal temperature measurable by the MLX
            aux += arr[i];
            n++;
        }
    }
    return (aux/(n*1.0));
}

double readTemperature(char command) {
    int dev = 0x5A<<1;
    int data_low = 0;
    int data_high = 0;
    int pec = 0;

    if (TEST)
        return (random(-40,120));

    i2c_start_wait(dev+I2C_WRITE);
    i2c_write(command);

  // read
    i2c_rep_start(dev+I2C_READ);
    data_low = i2c_readAck(); //Read 1 byte and then send ack
    data_high = i2c_readAck(); //Read 1 byte and then send ack
    pec = i2c_readNak();
    i2c_stop();

  //This converts high and low bytes together and processes temperature, MSB is a error bit and is ignored for temps
    double tempFactor = 0.02; // 0.02 degrees per LSB (measurement resolution of the MLX90614)
    double tempData = 0x0000; // zero out the data

  // This masks off the error bit of the high byte, then moves it left 8 bits and adds the low byte.
    tempData = (double)(((data_high & 0x007F) << 8) + data_low);
    tempData = (tempData * tempFactor)-0.01;
    return (tempData);
}

void transitionServoPosition(int from, int to) {
  Serial.println(from);
  Serial.println(to);
  if (from >= to) {
    for (int i = from ; i >= to ; i--) {
      myservo.write(i);
      delay(20);
    }
  } else {
    for (int i = from ; i <= to ; i++) {
      myservo.write(i);
      delay(20);
    }
  }
}

void setup(){
    Serial.begin(9600);
    Serial.println("servotemp starting...");

    i2c_init(); //Initialise the i2c bus
    PORTC = (1 << PORTC4) | (1 << PORTC5);//enable pullups

    myservo.attach(9);
}

void loop(){
    int wait_time = round(((float)WAIT_TIME*1000.0/(float)N_SAMPLES));
    for (int i = 0 ; i < N_STOPS ; i++) {
        old_pos = current_pos;
        current_pos = stop_positions[i];
        transitionServoPosition(old_pos, current_pos); 
        reset_measures(temp_amb);
        reset_measures(temp_obj);
        for (int j = 0 ; j < N_SAMPLES ; j++) {
            temp_amb[j] = readTemperature(T_AMB);
            temp_obj[j] = readTemperature(T_OBJ);
            if (DEBUG) {
              Serial.print("Waiting for ");
              Serial.print(wait_time/1000);
              Serial.println(" second(s)");
            }
            delay(wait_time);
        }
        send_results(current_pos, average(temp_amb), average(temp_obj));
    }
}
