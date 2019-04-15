#include <SPI.h>
#include "mcp_can.h"
#include <Wire.h>
#include <Adafruit_MLX90614.h>

#define canDelay 50
#define canOutputId 61
#define spiCsPin 9

MCP_CAN CAN(spiCsPin);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const int SPI_CS_PIN = 9;
unsigned long canLastSent = 0;
//Unsigned char acts as byte for message sending
float curr_temp = 0;
float amb_temp= 0;
int amb_arr[10];
int curr_arr[10];
int curr_avg = 0;
int amb_avg = 0;

int count = 0;

//funtion for setting a whole array to a single value
void reset_arr(int (&arr)[10],int temp)
{
  for(int i = 0; i < 10; i++)
  {
    arr[i] = temp;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mlx.begin();  
  START_INIT:

  if(CAN_OK == CAN.begin(CAN_500KBPS))                   // init can bus : baudrate = 500k
  {
      Serial.println("CAN BUS Shield init ok!");
  }
  else
  {
      Serial.println("CAN BUS Shield init fail");
      Serial.println("Init CAN BUS Shield again");
      delay(100);
      goto START_INIT;
  }
  
  // fill both arrays with the current ambient and rotor temperture
  curr_temp = mlx.readObjectTempC();
  amb_temp = mlx.readAmbientTempC();
  reset_arr(curr_arr,curr_temp);
  reset_arr(amb_arr,amb_temp);
  
}

void loop() {
  //make maximum temp array for curr to account for gaps in rotor
  for(int i = 0; i < 10; i++)
    {
      curr_temp = mlx.readObjectTempC();
      amb_temp = mlx.readAmbientTempC();
      curr_temp = curr_temp*100;
      amb_temp = amb_temp*100;
      amb_arr[i] = amb_temp;
      if(curr_temp > curr_arr[i]) curr_arr[i] = curr_temp;
      // count = count + 1;
    }
  
  
   if( millis()-canLastSent > canDelay)
    {
      amb_avg = 0;
      curr_avg = 0;
   
   // get average of temperture arrays
   for(int i = 0; i < 10; i++)
     {
      amb_avg = amb_avg + amb_arr[i]/10;
      curr_avg = curr_avg + curr_arr[i]/10;
     }
  
    //set message
    unsigned char canMessage[8] = {0, 0, 0, 0};
    canMessage[0] = curr_avg;
    canMessage[1] = curr_avg >> 8;
    canMessage[2] = amb_avg;
    canMessage[3] = amb_avg >> 8;

//    Serial.println(amb_avg);
//    Serial.println(canMessage[0]);
//    Serial.println(canMessage[1]);
 
    
    CAN.sendMsgBuf(canOutputId, 0, 8, canMessage);
    canLastSent = millis();

    // clear curr array to allow for temperture drops
    reset_arr(curr_arr,0);
    
    // For debugging
    // Serial.println(count);
    // count = 0;
  }
}
