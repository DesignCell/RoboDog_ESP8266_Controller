
//Read from Blynk App
BLYNK_WRITE(V0)
{
  Btn_Stop = param.asInt(); // assigning incoming value from pin V1 to a variable
}
BLYNK_WRITE(V1)
{
  Btn_State = param.asInt(); // assigning incoming value from pin V1 to a variable
}
BLYNK_WRITE(V3)
{
  Joy_X = param.asInt(); // assigning incoming value from pin V1 to a variable
}
BLYNK_WRITE(V4)
{
  Joy_Y = param.asInt(); // assigning incoming value from pin V1 to a variable
}
BLYNK_WRITE(V5)
{
  Joy_ZZ = param.asInt(); // assigning incoming value from pin V1 to a variable
}
BLYNK_WRITE(V6)
{
  Joy_Z = param.asInt(); // assigning incoming value from pin V1 to a variable
}
