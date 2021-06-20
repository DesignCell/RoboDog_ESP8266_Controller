/*
  Name: RoboDog_Controller
	Description: ESP8266 Wifi Controller for RoboDog
  Created:	2021.05.21
	Revised:	2021.06.20
  Author:   DesignCell (Jeff Underly)

*/

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define I2C_ADDR_FRONT 0
#define I2C_ADDR_REAR 2

#define SERIAL_DEBUG

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "GinyQKPRF1erjmoWGb6H-FU73tI-NSlB"; // RoboDog
bool isFirstConnect = true;
uint8_t blynk_cnt = 1;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "NetCell";
char pass[] = "Sveny;The100%BrownBeaner";

// Declarations
float Len_Length = 312.00; //mm
float Len_Width = 185.00; //mm
float Len_Upper = 165.00; //mm
float Len_Lower = 165.00; //mm

bool Btn_Stop = 0; //V0
bool Btn_State = 0; //V1
float SPEED; //V2
int8_t Spd_PWM; //V3
float Joy_X; //V5 +/-1000
float Joy_Y; //V6 +/-1000
float Joy_ZZ;//V7 +/-1000
float Joy_Z; //V8 200-300
uint8_t Lift_Z; //V9 0-60
uint8_t Kp;     //V10 0-255
uint8_t Ki;     //V11
uint8_t Kd;     //V12
uint8_t Offset_limit; //V16 0-60
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
uint8_t balance = 0;
float lean = 0;
int Leg_Active = 0;
int Leg_Is_Active = 0;
uint32_t Lift_Millis;

// Position Interval
uint32_t Pos_pMicros = 0;
uint8_t Pos_Hz = 75; //Hz :: 50Hz = 20,000us, 75Hz = 13,333us
uint32_t Pos_Interval = 1000000/Pos_Hz; //Microsecond method interval 
uint32_t Loop_time; // Loop time debug observation
uint32_t STM_Loop_Time[3]; // [0] & [2]



void setup() 
{
  // Debug console
  #ifdef SERIAL_DEBUG
  Serial.begin(250000);
  #endif

  Blynk.begin(auth, ssid, pass, IPAddress(192,168,0,8), 8080);

  Wire.begin();
  //Wire.setClock(400000L); //fast clock

  delay(500);
}

BLYNK_CONNECTED() // This function will run every time Blynk initial connection is established
{
  if (isFirstConnect) {
    Blynk.virtualWrite(V0,0); // Reset E-Stop 
    Blynk.virtualWrite(V1,0); // Reset Enabled State
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
  uint32_t  numI0 = STM_Loop_Time[0];
  uint32_t  numI2 = STM_Loop_Time[2];
  uint8_t   num000 = Leg_Active;
  uint8_t   num001 = Leg_Is_Active;
  uint8_t   num01 = Flag_1;
  uint8_t   num02 = Flag_2;
  uint8_t   num03 = Flag_3;
  uint8_t   num04 = Flag_4;
	int16_t 	num1X = Foot_Off_X[0];
	int16_t 	num1Y = Foot_Off_Y[0];
	int16_t 	num1Z = Foot_Off_Z[0];
  int16_t 	num1S = Leg_Angle_Setpoint_Shoulder[0];
  int16_t 	num1U = Leg_Angle_Setpoint_Upper[0];
  int16_t 	num1L = Leg_Angle_Setpoint_Lower[0]; 
  int16_t 	num2X = Foot_Off_X[1];
	int16_t 	num2Y = Foot_Off_Y[1];
	int16_t 	num2Z = Foot_Off_Z[1];
  int16_t 	num2S = Leg_Angle_Setpoint_Shoulder[1];
  int16_t 	num2U = Leg_Angle_Setpoint_Upper[1];
  int16_t 	num2L = Leg_Angle_Setpoint_Lower[1]; 
  int16_t 	num3X = Foot_Off_X[2];
	int16_t 	num3Y = Foot_Off_Y[2];
	int16_t 	num3Z = Foot_Off_Z[2];
  int16_t 	num3S = Leg_Angle_Setpoint_Shoulder[2];
  int16_t 	num3U = Leg_Angle_Setpoint_Upper[2];
  int16_t 	num3L = Leg_Angle_Setpoint_Lower[2]; 
  int16_t 	num4X = Foot_Off_X[3];
	int16_t 	num4Y = Foot_Off_Y[3];
	int16_t 	num4Z = Foot_Off_Z[3];
  int16_t 	num4S = Leg_Angle_Setpoint_Shoulder[3];
  int16_t 	num4U = Leg_Angle_Setpoint_Upper[3];
  int16_t 	num4L = Leg_Angle_Setpoint_Lower[3]; 


	sprintf(sPrint, "%d, %d, %d%d %d%d%d%d : %d,%d,%d, %d,%d,%d :: %d,%d,%d, %d,%d,%d :: %d,%d,%d, %d,%d,%d :: %d,%d,%d, %d,%d,%d", 
    numI0,numI2,num000,num001,num01,num02,num03,num04,
    num1X,num1Y,num1Z,num1S,num1U,num1L,
    num2X,num2Y,num2Z,num2S,num2U,num2L,
    num3X,num3Y,num3Z,num3S,num3U,num3L,
    num4X,num4Y,num4Z,num4S,num4U,num4L);


	//Print
	Serial.println(sPrint);
 
}

void Serial_Print__()
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

	sprintf(sPrint, "%d, %d, X%d, Y%d, Z%d, S%d, U%d, L%d, %d, %d", num1, num2, num3, num4, num5, num6, num7, num8, num9, num10);


	//Print
	Serial.println(sPrint);
 
}

void Serial_Print_()
{
	char sPrint[150];
  uint16_t  num1 = Loop_time;
	int16_t 	num2 = Flag_1;  
	int16_t 	num3 = Leg_Angle_Measured_Shoulder[0];
	int16_t 	num4 = Leg_Angle_Measured_Upper[0];
	int16_t 	num5 = Leg_Angle_Measured_Lower[0];
  int16_t 	num6 = Leg_Angle_Measured_Shoulder[1];
  int16_t 	num7 = Leg_Angle_Measured_Upper[1];
  int16_t 	num8 = Leg_Angle_Measured_Lower[1];
	int16_t 	num9 = Leg_Angle_Measured_Shoulder[2];
	int16_t 	num10 = Leg_Angle_Measured_Upper[2];
	int16_t 	num11 = Leg_Angle_Measured_Lower[2];
  int16_t 	num12 = Leg_Angle_Measured_Shoulder[3];
  int16_t 	num13 = Leg_Angle_Measured_Upper[3];
  int16_t 	num14 = Leg_Angle_Measured_Lower[3];

	sprintf(sPrint, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", num1, num2, num3, num4, num5, num6, num7, num8, num9, num10, num11, num12, num13, num14);


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
  lean = param.asInt();
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
BLYNK_WRITE(V10)
{
  Kp = param.asInt();
}
BLYNK_WRITE(V11)
{
  Ki = param.asInt();
}
BLYNK_WRITE(V12)
{
  Kd = param.asInt();
}
BLYNK_WRITE(V16)
{
  Offset_limit = param.asInt();
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

  Leg_Active = 0;
  Leg_Is_Active = 0;
  balance = 0;
  Flag_1 = 2;
  Flag_2 = 2;
  Flag_3 = 2;
  Flag_4 = 2;
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
  // Speed in mm/s through Hz multiplied by Joy precentage added to Offset.
  Off_X = Joy_X / 1000.0 * SPEED / Pos_Hz;
  Off_Y = Joy_Y / 1000.0 * SPEED / Pos_Hz;
  Off_ZZ = Joy_ZZ / 1000.0 * SPEED / Pos_Hz;

  /*
  Foot_Off_Z[0] = Off_Z;
  Foot_Off_Z[1] = Off_Z;
  Foot_Off_Z[2] = Off_Z;
  Foot_Off_Z[3] = Off_Z;
  */
}

bool Offsets_Limit(float X,float Y)
{
  if (sqrt(pow(X,2) + pow(Y,2)) < Offset_limit)
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
  Foot_Off_ZZ_X[0] = Foot_Hyp_Rel[0] * sin(Foot_Hyp_Angle[0] + Off_ZZr) - Foot_X_Rel[0];
  Foot_Off_ZZ_Y[0] = Foot_Y_Rel[0] - Foot_Hyp_Rel[0] * cos(Foot_Hyp_Angle[0] + Off_ZZr);

  Foot_X_Rel[1] = Len_Width * 0.5 - Foot_Off_X[1];
  Foot_Y_Rel[1] = Len_Length * 0.5 - Foot_Off_Y[1];
  Foot_Hyp_Rel[1] = sqrt(pow(Foot_X_Rel[1],2) + pow(Foot_Y_Rel[1],2));
  Foot_Hyp_Angle[1] = atan(Foot_X_Rel[1] / Foot_Y_Rel[1]);
  Foot_Off_ZZ_X[1] = Foot_X_Rel[1] - Foot_Hyp_Rel[1] * sin(Foot_Hyp_Angle[1] - Off_ZZr);
  Foot_Off_ZZ_Y[1] = Foot_Y_Rel[1] - Foot_Hyp_Rel[1] * cos(Foot_Hyp_Angle[1] - Off_ZZr);

  Foot_X_Rel[2] = Len_Width * 0.5 + Foot_Off_X[2];
  Foot_Y_Rel[2] = Len_Length * 0.5 + Foot_Off_Y[2];
  Foot_Hyp_Rel[2] = sqrt(pow(Foot_X_Rel[2],2) + pow(Foot_Y_Rel[2],2));
  Foot_Hyp_Angle[2] = atan(Foot_X_Rel[2] / Foot_Y_Rel[2]);
  Foot_Off_ZZ_X[2] = Foot_Hyp_Rel[2] * sin(Foot_Hyp_Angle[2] - Off_ZZr) - Foot_X_Rel[2];
  Foot_Off_ZZ_Y[2] = Foot_Hyp_Rel[2] * cos(Foot_Hyp_Angle[2] - Off_ZZr) - Foot_Y_Rel[2];

  Foot_X_Rel[3] = Len_Width * 0.5 - Foot_Off_X[3];
  Foot_Y_Rel[3] = Len_Length * 0.5 + Foot_Off_Y[3];
  Foot_Hyp_Rel[3] = sqrt(pow(Foot_X_Rel[3],2) + pow(Foot_Y_Rel[3],2));
  Foot_Hyp_Angle[3] = atan(Foot_X_Rel[3] / Foot_Y_Rel[3]);
  Foot_Off_ZZ_X[3] = Foot_X_Rel[3] - Foot_Hyp_Rel[3] * sin(Foot_Hyp_Angle[3] + Off_ZZr);
  Foot_Off_ZZ_Y[3] = Foot_Hyp_Rel[3] * cos(Foot_Hyp_Angle[3] + Off_ZZr) - Foot_Y_Rel[3];
  

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

void Balance_Pos(float lean) // Leans Right
{
  Foot_Off_X[0] += lean;
  Foot_Off_X[1] += lean;
  Foot_Off_X[2] += lean;
  Foot_Off_X[3] += lean;
}

void Balance_Neg(float lean)  // Leans Left
{
  Foot_Off_X[0] -= lean;
  Foot_Off_X[1] -= lean;
  Foot_Off_X[2] -= lean;
  Foot_Off_X[3] -= lean;
}

// Determine if the Leg is close to setpoint position before moving on.
bool In_Position(int Leg)
{
  uint16_t degrees_off = 30; // 1.5d
  if (abs(Leg_Angle_Measured_Shoulder[Leg - 1] -  Leg_Angle_Setpoint_Shoulder[Leg-1]) <= degrees_off)
  {
    if (abs(Leg_Angle_Measured_Upper[Leg - 1] -  Leg_Angle_Setpoint_Upper[Leg-1]) <= degrees_off)
    {
      if (abs(Leg_Angle_Measured_Lower[Leg - 1] -  Leg_Angle_Setpoint_Lower[Leg-1]) <= degrees_off)
      {
        return true;
      }
    }
  }
  return false;
}

void Foot_Lift()
{
  switch (Leg_Active)
  {
    case 0: // Set Leg not at zero to active, most likely Leg 1
      if (int(Foot_Off_X[0]) != 0 || int(Foot_Off_Y[0]) != 0)
      {
        Leg_Active = 1;
        break;
      }
      if (int(Foot_Off_X[1]) != 0 || int(Foot_Off_Y[1]) != 0)
      {
        Leg_Active = 2;
        break;
      }
      if (int(Foot_Off_X[2]) != 0 || int(Foot_Off_Y[2]) != 0)
      {
        Leg_Active = 3;
        break;
      }
      if (int(Foot_Off_X[3]) != 0 || int(Foot_Off_Y[3]) != 0)
      {
        Leg_Active = 4;
        break;
      }
      Leg_Active = 0;
      break;

    case 1: // Front Left
      if (Leg_Is_Active == 0)
      {
        Flag_1 = 0; // Foot Up
        Leg_Is_Active = 1; // Set A leg is active flag
        if (Foot_Off_X[0] <= 0) // Leaning Left?
        {
          balance = 1; // Set Ballance Flag to know if return is needed.
          Balance_Pos(lean); // Balance right
        }
        Foot_Off_Z[0] -= Lift_Z; // Lift Foot
        break;
      }
      if (Flag_1 == 0 && In_Position(1) == true) 
      {
        Flag_1 = 1; // Foot Relocating
        if (Joy_X == 0 && Joy_Y == 0 && Joy_ZZ == 0)
        {
          Foot_Off_X[0] = 0;
          Foot_Off_Y[0] = 0;
        } else
        {
          // Tried to relocate foot opposite of step from location. 
          // Needs work with Yaw movements.
          Foot_Off_X[0] = 0; //-Foot_Off_X[0];
          Foot_Off_Y[0] = 0; //-Foot_Off_Y[0];
        }
        break;
      }
      if (Flag_1 == 1 && In_Position(1) == true)
      {
        if (balance == 1)
        {
          balance = 0; // Reset balance flag
          Balance_Neg(lean); // Lean Left
        }
        Foot_Off_Z[0] += Lift_Z; // Lower Foot
        Flag_1 = 2; // Foot Down
        break;
      }
      if (Flag_1 == 2 && In_Position(1) == true)
      {
        Leg_Active +=1; // Next Foot
        Leg_Is_Active = 0; // Reset Leg Active Flag
      }
      break;

    case 2: // Front Right
      if (Leg_Is_Active == 0)
      {
        Flag_2 = 0; // Foot Up
        Leg_Is_Active = 1; // Set Leg is active flag
        if (Foot_Off_X[1] >= 0) // Leaning Right?
        {
          balance = 1; // Set Ballance Flag to know if return is needed.
          Balance_Neg(lean); // Lean Left
        }
        Foot_Off_Z[1] -= Lift_Z; // Lift Foot
        break;
      }
      if (Flag_2 == 0 && In_Position(2) == true) 
      {
        Flag_2 = 1; // Foot Relocating
        if (Joy_X == 0 && Joy_Y == 0 && Joy_ZZ == 0)
        {
          Foot_Off_X[1] = 0;
          Foot_Off_Y[1] = 0;
        } else
        {
          // Tried to relocate foot opposite of step from location. 
          // Needs work with Yaw movements.
          Foot_Off_X[1] = 0; //-Foot_Off_X[1];
          Foot_Off_Y[1] = 0; //-Foot_Off_Y[1];
        }
        break;
      }
      if (Flag_2 == 1 && In_Position(2) == true)
      {
        if (balance == 1)
        {
          balance = 0; // Set Ballance Flag to know if return is needed.
          Balance_Pos(lean);  // Lean Right
        }
        Foot_Off_Z[1] += Lift_Z; // Lower Foot
        Flag_2 = 2; // Foot Down
        break;
      }
      if (Flag_2 == 2 && In_Position(2) == true)
      {
        Leg_Active +=1; // Next Foot
        Leg_Is_Active = 0; // Reset Leg Active Flag
      }
      break;

    case 3: // Rear Left
      if (Leg_Is_Active == 0)
      {
        Flag_3 = 0; // Foot Up
        Leg_Is_Active = 1; // Set A leg is active flag
        if (Foot_Off_X[2] <= 0) // Leaning Left?
        {
          balance = 1; // Set Ballance Flag to know if return is needed.
          Balance_Pos(lean); // Balance right
        }
        Foot_Off_Z[2] -= Lift_Z; // Lift Foot
        break;
      }
      if (Flag_3 == 0 && In_Position(3) == true) 
      {
        Flag_3 = 1; // Foot Relocating
        if (Joy_X == 0 && Joy_Y == 0 && Joy_ZZ == 0)
        {
          Foot_Off_X[2] = 0;
          Foot_Off_Y[2] = 0;
        } else
        {
          // Tried to relocate foot opposite of step from location. 
          // Needs work with Yaw movements.
          Foot_Off_X[2] = 0; //-Foot_Off_X[2];
          Foot_Off_Y[2] = 0; //-Foot_Off_Y[2];
        }
        break;
      }
      if (Flag_3 == 1 && In_Position(3) == true)
      {
        if (balance == 1)
        {
          balance = 0; // Reset balance flag
          Balance_Neg(lean); // Lean Left
        }
        Foot_Off_Z[2] += Lift_Z; // Lower Foot
        Flag_3 = 2; // Foot Down
        break;
      }
      if (F