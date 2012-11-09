#include <i2cmaster.h>
#include <Servo.h> 

#define T_AMB 0x06
#define T_OBJ 0x07
#define WAIT_TIME 5 // number of minutes between each measure
#define N_SAMPLES 5 // number of samples to average between to measures
#define DEBUG false
#define N_STOPS 3
int stop_positions[N_STOPS] = {0, 85, 169}; // Angles at which the servo will stop to measure the temperature

float temp_amb[N_SAMPLES];
float temp_obj[N_SAMPLES];

Servo myservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 

int pos = 0;    // variable to store the servo position 

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
    strncpy(buffer+5, float2s(amb), 7);
    strncpy(buffer+12, ",", 1);
    strncpy(buffer+13, float2s(obj), 7);
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

    if (DEBUG)
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

void setup(){
    Serial.begin(9600);
    Serial.println("servotemp starting...");

    i2c_init(); //Initialise the i2c bus
    PORTC = (1 << PORTC4) | (1 << PORTC5);//enable pullups

    myservo.attach(9);
    Serial.println('Still here');
}

void loop(){
    int current_pos = 0;
    for (int i = 0 ; i < N_STOPS ; i++) {
        current_pos = stop_positions[i];
        myservo.write(current_pos);
        delay(1000); // wait for 1 second to give the servo enough time to move to its position
        reset_measures(temp_amb);
        reset_measures(temp_obj);
        for (int j = 0 ; j < N_SAMPLES ; j++) {
            temp_amb[j] = readTemperature(T_AMB);
            temp_obj[j] = readTemperature(T_OBJ);
            delay(round(WAIT_TIME/N_SAMPLES)*60000);
        }
        send_results(current_pos, average(temp_amb), average(temp_obj));
    }
}
