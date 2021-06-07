/*
  Name: RoboDog_Controller
	Description: ESP8266 Wifi Controller for RoboDog
  Created:	2021.05.21
	Revised:	  
  Author:     DesignCell

*/

//#define BLYNK_PRINT Serial // Defines the object that is used for printing
//#define BLYNK_DEBUG        // Optional, this enables more detailed prints
//#define BLYNK_MSG_LIMIT      100

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>




#define I2C_ADDR_FRONT 0
#define I2C_ADDR_REAR 2


// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "TOKEN"; // RoboDog
bool isFirstConnect = true;
uint8_t blynk_cnt = 1;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "WIFI_SSID";
char pass[] = "WIFI_PASSWORD";

// Declarations
float Len_Length = 312.00; //mm
float Len_Width = 185.00; //mm
float Len_Upper = 165.00; //mm
float Len_Lower = 165.00; //mm

bool Btn_Stop = 0; //V0
bool Btn_State = 0; //V1
float SPEED; //V2
float Joy_X; //V5 +/-500
float Joy_Y; //V6 +/-500
float Joy_ZZ;//V7 +/-500
float Joy_Z; //V8 +/-500
uint8_t Lift_Z; //V9 0-60
uint8_t Off_Lim; //V16 0-60
uint8_t Off_Lift; //V17 0-60

float Off_X = 0;
float Off_Y = 0;
float Off_Z;
float Off_ZZ = 0;

float Foot_Off_X [4] = { };
float Foot_Off_Y [4] = { };
float Foot_Off_Z [4] = { };
float Foot_X_Rel[4];
float Foot_Y_Rel[4];
float Foot_Hyp_Rel[4];
float Foot_Hyp_Angle[4];

int16_t Leg_Angle_Setpoint_Shoulder[4];
int16_t Leg_Angle_Setpoint_Upper[4];
int16_t Leg_Angle_Setpoint_Lower[4];

int16_t Leg_Angle_Measured_Shoulder[4];
int16_t Leg_Angle_Measured_Upper[4];
int16_t Leg_Angle_Measured_Lower[4];

//Foot On Ground Flags:
// 0 = Up
// 1 = Relocating
// 2 = Down
uint8_t Flag_1 = 2;
uint8_t Flag_2 = 2;
uint8_t Flag_3 = 2;
uint8_t Flag_4 = 2;
int Leg_Active = 1;
int Leg_Is_Active = 0;
uint32_t Lift_Millis;

// Position Interval
uint32_t Pos_pMicros = 0;
uint8_t Pos_Hz = 50; //Hz :: 50Hz = 20,000us, 75Hz = 13,333us
uint32_t Pos_Interval = 1000000/Pos_Hz; //Microsecond method interval 
uint32_t Loop_time; // Loop time debug observation


void setup() 
{
  // Debug console
  Serial.begin(250000);

  Blynk.begin(auth, ssid, pass, IPAddress(192,168,0,8), 8080);

  Wire.begin();
  //Wire.setClock(400000L); //fast clock


  delay(500);
}

BLYNK_CONNECTED() // This function will run every time Blynk initial connection is established
{
  if (isFirstConnect) {
    Blynk.virtualWrite(V0,0); // Reset E-Stop
    Blynk.syncAll();
    isFirstConnect = false;

    Foot_Off_Z[0] = Off_Z;
    Foot_Off_Z[1] = Off_Z;
    Foot_Off_Z[2] = Off_Z;
    Foot_Off_Z[3] = Off_Z;
  }
}

void Serial_Print()
{
	char sPrint[150];
  uint16_t  num1 = Loop_time;
	int16_t 	num2 = Flag_1;  
	int16_t 	num3 = Foot_Off_X[0];
	int16_t 	num4 = Foot_Off_Y[0];
	int16_t 	num5 = Foot_Off_Z[0];
  int16_t 	num6 = Leg_Angle_Setpoint_Shoulder[0];
  int16_t 	num7 = Leg_Angle_Setpoint_Upper[0];
  int16_t 	num8 = Leg_Angle_Setpoint_Lower[0];
  int16_t 	num9 = Leg_Angle_Measured_Shoulder[0];
  int16_t 	num10 =Leg_Angle_Measured_Upper[0];

	sprintf(sPrint, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", num1, num2, num3, num4, num5, num6, num7, num8, num9, num10);


	//Print
	Serial.println(sPrint);
 
}

// Receive from Blynk App
BLYNK_WRITE(V0)
{
  Btn_Stop = param.asInt(); 
}
BLYNK_WRITE(V1)
{
  Btn_State = param.asInt();
}
BLYNK_WRITE(V2)
{
  SPEED = param.asInt();
}
BLYNK_WRITE(V3)
{
  //UNUSED = param.asInt();
}
BLYNK_WRITE(V4)
{
  //UNUSED = param.asInt();
}
BLYNK_WRITE(V5)
{
  Joy_X = param.asInt(); 
}
BLYNK_WRITE(V6)
{
  Joy_Y = param.asInt(); 
}
BLYNK_WRITE(V7)
{
  Joy_ZZ = param.asInt();
}
BLYNK_WRITE(V8)
{
  Joy_Z = param.asInt(); 
}
BLYNK_WRITE(V9)
{
  Lift_Z = param.asInt();
}
BLYNK_WRITE(V16)
{
  Off_Lim = param.asInt();
}
BLYNK_WRITE(V17)
{
  Foot_Off_X[0] = 0;
  Foot_Off_X[1] = 0;
  Foot_Off_X[2] = 0;
  Foot_Off_X[3] = 0;
  Foot_Off_Y[0] = 0;
  Foot_Off_Y[1] = 0;
  Foot_Off_Y[2] = 0;
  Foot_Off_Y[3] = 0;
  Foot_Off_Z[0] = Off_Z;
  Foot_Off_Z[1] = Off_Z;
  Foot_Off_Z[2] = Off_Z;
  Foot_Off_Z[3] = Off_Z;
}
BLYNK_WRITE(V18)
{
  Off_Z = param.asInt(); 
}
BLYNK_WRITE(V19)
{
  Off_Lift = param.asInt(); 
}

void Offsets()
{
  // *******************************************  mm/s adder here
  Off_X = Joy_X * (1 / SPEED);
  Off_Y = Joy_Y * (1 / SPEED);
  Off_ZZ = Joy_ZZ * (1 / SPEED);

  Foot_Off_Z[0] = Off_Z;
  Foot_Off_Z[1] = Off_Z;
  Foot_Off_Z[2] = Off_Z;
  Foot_Off_Z[3] = Off_Z;
}

bool Offsets_Limit(float X,float Y)
{
  if (sqrt(pow(X,2) + pow(Y,2)) < Off_Lim)
  {
    return true;
  } else
  {
    return false;
  }

}

bool Offsets_Lift(float X,float Y)
{
  if (sqrt(pow(X,2) + pow(Y,2)) > Off_Lift)
  {
    return true;
  } else
  {
    return false;
  }

}

void Offsets_XY()
{
  if (Offsets_Limit(Foot_Off_X[0] + Off_X,Foot_Off_Y[0] + Off_Y) == true) {
    if (Offsets_Limit(Foot_Off_X[1] + Off_X,Foot_Off_Y[1] + Off_Y) == true) {
      if (Offsets_Limit(Foot_Off_X[2] + Off_X,Foot_Off_Y[2] + Off_Y) == true) {
        if (Offsets_Limit(Foot_Off_X[3] + Off_X,Foot_Off_Y[3] + Off_Y) == true) {
          if (Flag_1 == 2){
            Foot_Off_X[0] += Off_X;
            Foot_Off_Y[0] += Off_Y;
          }
          if (Flag_2 == 2){
            Foot_Off_X[1] += Off_X;
            Foot_Off_Y[1] += Off_Y;
          }
          if (Flag_3== 2){
            Foot_Off_X[2] += Off_X;
            Foot_Off_Y[2] += Off_Y;
          }
          if (Flag_4 == 2){
            Foot_Off_X[3] += Off_X;
            Foot_Off_Y[3] += Off_Y;
          }
        }
      }
    }
  }
}

void Offsets_ZZ()
{

  float Off_ZZr = radians(Off_ZZ);
  float Foot_Off_ZZ_X[4] = { };
  float Foot_Off_ZZ_Y[4] = { };

  Foot_X_Rel[0] = Len_Width * 0.5 + Foot_Off_X[0];
  Foot_Y_Rel[0] = Len_Length * 0.5 - Foot_Off_Y[0];
  Foot_Hyp_Rel[0] = sqrt(pow(Foot_X_Rel[0],2) + pow(Foot_Y_Rel[0],2));
  Foot_Hyp_Angle[0] = atan(Foot_X_Rel[0] / Foot_Y_Rel[0]);
  Foot_Off_ZZ_X[0] += Foot_Hyp_Rel[0] * sin(Foot_Hyp_Angle[0] + Off_ZZr) - Foot_X_Rel[0];
  Foot_Off_ZZ_Y[0] += Foot_Y_Rel[0] - Foot_Hyp_Rel[0] * cos(Foot_Hyp_Angle[0] + Off_ZZr);

  Foot_X_Rel[1] = Len_Width * 0.5 - Foot_Off_X[1];
  Foot_Y_Rel[1] = Len_Length * 0.5 - Foot_Off_Y[1];
  Foot_Hyp_Rel[1] = sqrt(pow(Foot_X_Rel[1],2) + pow(Foot_Y_Rel[1],2));
  Foot_Hyp_Angle[1] = atan(Foot_X_Rel[1] / Foot_Y_Rel[1]);
  Foot_Off_ZZ_X[1] += Foot_X_Rel[1] - Foot_Hyp_Rel[1] * sin(Foot_Hyp_Angle[1] - Off_ZZr);
  Foot_Off_ZZ_Y[1] += Foot_Y_Rel[1] - Foot_Hyp_Rel[1] * cos(Foot_Hyp_Angle[1] - Off_ZZr);

  Foot_X_Rel[2] = Len_Width * 0.5 + Foot_Off_X[2];
  Foot_Y_Rel[2] = Len_Length * 0.5 + Foot_Off_Y[2];
  Foot_Hyp_Rel[2] = sqrt(pow(Foot_X_Rel[2],2) + pow(Foot_Y_Rel[2],2));
  Foot_Hyp_Angle[2] = atan(Foot_X_Rel[2] / Foot_Y_Rel[2]);
  Foot_Off_ZZ_X[2] += Foot_Hyp_Rel[2] * sin(Foot_Hyp_Angle[2] - Off_ZZr) - Foot_X_Rel[2];
  Foot_Off_ZZ_Y[2] += Foot_Hyp_Rel[2] * cos(Foot_Hyp_Angle[2] - Off_ZZr) - Foot_Y_Rel[2];

  Foot_X_Rel[3] = Len_Width * 0.5 - Foot_Off_X[3];
  Foot_Y_Rel[3] = Len_Length * 0.5 + Foot_Off_Y[3];
  Foot_Hyp_Rel[3] = sqrt(pow(Foot_X_Rel[3],2) + pow(Foot_Y_Rel[3],2));
  Foot_Hyp_Angle[3] = atan(Foot_X_Rel[3] / Foot_Y_Rel[3]);
  Foot_Off_ZZ_X[3] += Foot_X_Rel[3] - Foot_Hyp_Rel[3] * sin(Foot_Hyp_Angle[3] + Off_ZZr);
  Foot_Off_ZZ_Y[3] += Foot_Hyp_Rel[3] * cos(Foot_Hyp_Angle[3] + Off_ZZr) - Foot_Y_Rel[3];
  

  if (Offsets_Limit(Foot_Off_X[0] + Foot_Off_ZZ_X[0],Foot_Off_Y[0] + Foot_Off_ZZ_Y[0]) == true) {
    if (Offsets_Limit(Foot_Off_X[1] + Foot_Off_ZZ_X[1],Foot_Off_Y[1] + Foot_Off_ZZ_Y[1]) == true) {
      if (Offsets_Limit(Foot_Off_X[2] + Foot_Off_ZZ_X[2],Foot_Off_Y[2] + Foot_Off_ZZ_Y[2]) == true) {
        if (Offsets_Limit(Foot_Off_X[3] + Foot_Off_ZZ_X[3],Foot_Off_Y[3] + Foot_Off_ZZ_Y[3]) == true) 
          {
            if (Flag_1 == 2){
              Foot_Off_X[0] += Foot_Off_ZZ_X[0];
              Foot_Off_Y[0] += Foot_Off_ZZ_Y[0];
            }
            if (Flag_2 == 2){
              Foot_Off_X[1] += Foot_Off_ZZ_X[1];
              Foot_Off_Y[1] += Foot_Off_ZZ_Y[1];
            }
            if (Flag_3== 2){
              Foot_Off_X[2] += Foot_Off_ZZ_X[2];
              Foot_Off_Y[2] += Foot_Off_ZZ_Y[2];
            }
            if (Flag_4 == 2){
              Foot_Off_X[3] += Foot_Off_ZZ_X[3];
              Foot_Off_Y[3] += Foot_Off_ZZ_Y[3];
            }
          }
        }
      }
    }
  

}

// ********************************************** Rewrite
void Foot_Lift()
{

  switch (Leg_Active)
  {
    case 0:
      // Thinking to set leg_active as 0 when idel for +5sec? Then pick furtest from 0,0
      break;
    case 1:
      if (Leg_Is_Active == 0 && Offsets_Lift(Foot_Off_X[0],Foot_Off_Y[0]) == true)
      {
        Flag_1 = 0;
        Leg_Is_Active = 1;
        Foot_Off_Z[0] -= Lift_Z;
        Lift_Millis = millis();
      }
      if (Flag_1 == 0 && Lift_Millis > 2500) // Add if in 
      {
        Foot_Off_X[0] = -Foot_Off_X[0];
        Foot_Off_Y[0] = -Foot_Off_Y[0];
        Flag_1 = 1;
        Lift_Millis = millis();
      }
      if (Flag_1 == 1 && Lift_Millis > 4500)
      {
        Foot_Off_Z[0] -= Lift_Z;
        Flag_1 = 2;
        Lift_Millis = millis();
        Leg_Active +=1;
      }
      break;
    case 2:
      if (Leg_Is_Active == 0 && Offsets_Lift(Foot_Off_X[1],Foot_Off_Y[1]) == true)
      {
        Flag_2 = 0;
        Leg_Is_Active = 1;
        Foot_Off_Z[1] -= Lift_Z;
        Lift_Millis = millis();
      }
      if (Flag_2 == 0 && Lift_Millis > 2500) // Add if in 
      {
        Foot_Off_X[1] = -Foot_Off_X[1];
        Foot_Off_Y[1] = -Foot_Off_Y[1];
        Flag_2 = 1;
        Lift_Millis = millis();
      }
      if (Flag_2 == 1 && Lift_Millis > 4500)
      {
        Foot_Off_Z[1] -= Lift_Z;
        Flag_2 = 2;
        Lift_Millis = millis();
        Leg_Active +=1;
      }
      break;
    case 3:
      if (Leg_Is_Active == 0 && Offsets_Lift(Foot_Off_X[2],Foot_Off_Y[2]) == true)
      {
        Flag_3 = 0;
        Leg_Is_Active = 1;
        Foot_Off_Z[2] -= Lift_Z;
        Lift_Millis = millis();
      }
      if (Flag_3 == 0 && Lift_Millis > 2500) // Add if in 
      {
        Foot_Off_X[2] = -Foot_Off_X[2];
        Foot_Off_Y[2] = -Foot_Off_Y[2];
        Flag_3 = 1;
        Lift_Millis = millis();
      }
      if (Flag_3 == 1 && Lift_Millis > 4500)
      {
        Foot_Off_Z[2] -= Lift_Z;
        Flag_3 = 2;
        Lift_Millis = millis();
        Leg_Active +=1;
      }
      break;
    case 4:
      if (Leg_Is_Active == 0 && Offsets_Lift(Foot_Off_X[3],Foot_Off_Y[3]) == true)
      {
        Flag_4 = 0;
        Leg_Is_Active = 1;
        Foot_Off_Z[3] -= Lift_Z;
        Lift_Millis = millis();
      }
      if (Flag_4 == 0 && Lift_Millis > 2500) // Add if in 
      {
        Foot_Off_X[3] = -Foot_Off_X[3];
        Foot_Off_Y[3] = -Foot_Off_Y[3];
        Flag_4 = 1;
        Lift_Millis = millis();
      }
      if (Flag_4 == 1 && Lift_Millis > 4500)
      {
        Foot_Off_Z[3] -= Lift_Z;
        Flag_4 = 2;
        Lift_Millis = millis();
        Leg_Active +=1;
      }
      break;
    default:
      Leg_Active = 1;
      break;
  }
  if (Flag_1 == 2 && Flag_2 == 2 && Flag_3 == 2 && Flag_4 == 2 && Lift_Millis > 500)
  {
    Leg_Is_Active = 0;
  }
  if (Leg_Is_Active == 0 && Lift_Millis > 5000)
  {
    //Leg_Active = 0;
  }

}

        

void LegKinematics(int Leg)
{
  // (+) Foot_X, Foot left, robot body right  
  // (+) Foot_Y, Foot toward rear, robot body forward
  // (+) Foot_Z, Foot down, robot body up

  float Foot_Z1;
  float Len_Leg;
  
  float Angle_Upper_Off_Y;
  float Angle_Upper_abs;

  float Leg_Angle_Shoulder_f;
  float Leg_Angle_Lower_f;
  float Leg_Angle_Upper_f;

  // Foot_Off_X[Leg]
  // Foot_Off_Y[Leg]
  // Foot_Off_Z[Leg]

  Foot_Z1 = sqrt(pow(Foot_Off_Z[Leg],2) + pow(Foot_Off_X[Leg],2));
  Leg_Angle_Shoulder_f = degrees(atan(Foot_Off_X[Leg] / Foot_Off_Z[Leg])); // Zero is vertical

  Len_Leg = sqrt(pow(Foot_Off_Y[Leg],2) + pow(Foot_Z1,2));
  Angle_Upper_Off_Y = degrees(atan(Foot_Off_Y[Leg] / Foot_Z1)); // Zero is vertical

  // Angle_Upper measures Len_Leg to Len_Upper.
  Angle_Upper_abs = degrees(acos((pow(Len_Leg,2) + pow(Len_Upper,2) - pow(Len_Lower,2)) / (2 * Len_Leg * Len_Upper)));
  Leg_Angle_Lower_f = degrees(acos((pow(Len_Upper,2) + pow(Len_Upper,2) - pow(Len_Leg,2)) / (2 * Len_Upper * Len_Lower)));

  // Angle Upper Adjusted referenced from shoulder axis (horizontal)
  Leg_Angle_Upper_f = 90 - Angle_Upper_Off_Y - Angle_Upper_abs;

  Leg_Angle_Setpoint_Shoulder[Leg] = int(Leg_Angle_Shoulder_f * 10);
  Leg_Angle_Setpoint_Upper[Leg] = int(Leg_Angle_Upper_f * 10);
  Leg_Angle_Setpoint_Lower[Leg] = int(Leg_Angle_Lower_f * 10);


}

void Controller_Send(int I2C_ADDR)
{

  Wire.beginTransmission(I2C_ADDR);

  byte bitArray[2];
  bitArray[0] = ((Leg_Angle_Setpoint_Shoulder[0]) >> 8) & 0xFF;
  bitArray[1] = (Leg_Angle_Setpoint_Shoulder[0]) & 0xFF;
  Wire.write(bitArray, 2);
  bitArray[0] = (Leg_Angle_Setpoint_Upper[0] >> 8) & 0xFF;
  bitArray[1] = Leg_Angle_Setpoint_Upper[0] & 0xFF;
  Wire.write(bitArray, 2);
  bitArray[0] = (Leg_Angle_Setpoint_Lower[0] >> 8) & 0xFF;
  bitArray[1] = Leg_Angle_Setpoint_Lower[0] & 0xFF;
  Wire.write(bitArray, 2);

  bitArray[0] = (Leg_Angle_Setpoint_Shoulder[1] >> 8) & 0xFF;
  bitArray[1] = Leg_Angle_Setpoint_Shoulder[1] & 0xFF;
  Wire.write(bitArray, 2);
  bitArray[0] = (Leg_Angle_Setpoint_Upper[1] >> 8) & 0xFF;
  bitArray[1] = Leg_Angle_Setpoint_Upper[1] & 0xFF;
  Wire.write(bitArray, 2);
  bitArray[0] = (Leg_Angle_Setpoint_Lower[1] >> 8) & 0xFF;
  bitArray[1] = Leg_Angle_Setpoint_Lower[1] & 0xFF;
  Wire.write(bitArray, 2);
  Wire.endTransmission();
}

void Controller_Read(int I2C_ADDR)
{
  int bytes = 12;
  int available = Wire.requestFrom(I2C_ADDR,bytes);
  if (available == bytes)
  {
    Leg_Angle_Measured_Shoulder[0] = Wire.read() << 8 | Wire.read();
    Leg_Angle_Measured_Upper[0] = Wire.read() << 8 | Wire.read();
    Leg_Angle_Measured_Lower[0] = Wire.read() << 8 | Wire.read();
    Leg_Angle_Measured_Shoulder[1] = Wire.read() << 8 | Wire.read();
    Leg_Angle_Measured_Upper[1] = Wire.read() << 8 | Wire.read();
    Leg_Angle_Measured_Lower[1] = Wire.read() << 8 | Wire.read();
  }
  Wire.endTransmission();
}

void loop() {
  uint32_t p_micros = micros();
  Blynk.run();


  // Position Interval
	uint32_t Pos_cMicros = micros();
	if (Pos_cMicros - Pos_pMicros > Pos_Interval)
	{
		Pos_pMicros = Pos_cMicros;	//Reset time interval
    
    Offsets();
    Offsets_XY();
    Offsets_ZZ();
    
    //Foot_Lift();
    
    for (int cnt = 0; cnt < 4; cnt++)
    {
      LegKinematics(cnt);
    }
 

    Controller_Send(I2C_ADDR_FRONT);
    //Controller_Send(2);
    Controller_Read(I2C_ADDR_FRONT);
    //Controller_Read(2);


    Serial_Print();
    
    uint32_t c_micros = micros();
    Loop_time = c_micros - p_micros;
  }

}