#include <SPI.h>
#include "mcp_can.h"
#include <Wire.h>
#include <Adafruit_MLX90614.h>

#define CAN_BTS_OUT_ID 0x51   // CAN ID Out
#define canDelay 50
#define canOutputId 51
#define spiCsPin 9

MCP_CAN CAN(spiCsPin);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const int SPI_CS_PIN = 9;
unsigned long canLastSent = 0;
//Unsigned char acts as byte for message sending
int curr_temp = 0;
int amb_temp= 0;
int amb_arr[10];
int curr_arr[10];
int curr_avg = 0;
int amb_avg = 0;

//funtion for setting a whole array to single value
int[10] reset_arr(int temp)
{
  int[10] arr;
  for(i = 0; i < 10; i++)
  {
    arr[i] = temp;
  }
  return arr;
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
  curr_arr = reset_arr(curr_temp);
  amb_arr = reset_arr(amb_temp);
  
}

void loop() {
  curr_temp = mlx.readObjectTempC();
  amb_temp = mlx.readAmbientTempC();

  //make maximum temp array for curr to account for gaps in rotor
  for(i = 0; i < 10; i++)
    {
      if(curr_temp > curr_arr[i]) cur_arr[i] = curr_temp;
    }
  
  
   if( millis()-canLastSent > canDelay)
    {
      amb_avg = 0;
      curr_avg = 0;
   
   // get average of temperture arrays
   for(i = 0; i < 10; i++)
     {
      amb_avg = amb_avg + amb_avg[i]/10;
      curr_avg = curr_avg + curr_avg[i]/10;
     }
  
    //set message
    unsigned char canMessage[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    canMessage[0] = curr_avg;
    canMessage[1] = curr_avg >> 8;
    canMessage[2] = amb_avg;
    canMessage[3] = amb_avg >> 8;
    
    //send out AMS/IMD status as 8 bit CANOut char array
    CAN.sendMsgBuf((canOutputId, 0, 8, canMessage));
    canLastSent = millis();

    // clear curr array to allow for temperture drops
    curr_arr = reset_arr(curr_temp);

  }
}
