#include <ADC.h>

const int  NMAX0 = 50;
const int  NMAX1 = 50;

const int readPin0 = A0; // ADC0
const int readPin1 = A3; // ADC1
const int LED = 12; // 

ADC *adc = new ADC(); // adc object
String inputString = "";
char lastRecvd;
char intbuf[32];
const int buffLength = 32;  
char inputBuff[buffLength];
int buffPointer = 0;
String newSettingString;
boolean prevSerialAvailableState = false;
int ledCount = 0;
int ledCountMax = 10000;


int value0A[NMAX0];
int value1A[NMAX1];
int value0;
int value1;
int ipt0p = 0;
int ipt1p = 0;
int ipt0m = 0;
int ipt1m = 0;
int timeOffset0 = 20;
int timeOffset1 = 10;
int maxAcValue0 = 260;
int maxAcValue1 = 260;
int gain0 = 0;
int gain1 = 0;
int dacOffset0 = 488;
int dacOffset1 = 484;
int sign0 = 1;
int sign1 = 1;
int chanOn0 = 0;
int chanOn1 = 0;

void setup() 
{
  Serial.begin(9600);
  pinMode(readPin0, INPUT);
  pinMode(readPin1, INPUT);
  pinMode(LED, OUTPUT);
  analogWriteResolution(10);
  adc->setAveraging(1); // set number of averages
  adc->setResolution(10); // set bits of resolution
//  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED); // change the conversion speed
//  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED); // change the sampling speed
//  adc->enableInterrupts(ADC_0);
  adc->startContinuous(readPin0, ADC_0);
  adc->setAveraging(1, ADC_1); // set number of averages
  adc->setResolution(10, ADC_1); // set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_1); // change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_1); // change the sampling speed
//  adc->enableInterrupts(ADC_1);
  adc->startContinuous(readPin1, ADC_1);

  ipt0p = 0;
  for (int ii = 0; ii < NMAX0; ++ii) value0A[ii] = 0;

  ipt1p = 0;
  for (int ii = 0; ii < NMAX1; ++ii) value1A[ii] = 0;
  delay(1000);
  digitalWrite(LED, HIGH);
  prevSerialAvailableState = false;
}

void loop() 
{ 
  readSerial();
  if (inputString.length() > 0)
  {
   if (inputString.indexOf("timeOffset0") >= 0) timeOffset0 = newIntSetting("timeOffset0", inputString);
   if (inputString.indexOf("timeOffset1") >= 0) timeOffset1 = newIntSetting("timeOffset1", inputString);
   if (inputString.indexOf("sign0") >= 0) sign0 = newIntSetting("sign0", inputString);
   if (inputString.indexOf("sign1") >= 0) sign1 = newIntSetting("sign1", inputString);
   if (inputString.indexOf("dacOffset0") >= 0) dacOffset0 = newIntSetting("dacOffset0", inputString);
   if (inputString.indexOf("dacOffset1") >= 0) dacOffset1 = newIntSetting("dacOffset1", inputString);
   if (inputString.indexOf("maxAcValue0") >= 0) maxAcValue0 = newIntSetting("maxAcValue0", inputString);
   if (inputString.indexOf("maxAcValue1") >= 0) maxAcValue1 = newIntSetting("maxAcValue1", inputString);
   if (inputString.indexOf("timeOffset0") >= 0) timeOffset0 = newIntSetting("timeOffset0", inputString);
   if (inputString.indexOf("timeOffset1") >= 0) timeOffset1 = newIntSetting("timeOffset1", inputString);
   if (inputString.indexOf("gain0") >= 0) gain0 = newIntSetting("gain0", inputString);
   if (inputString.indexOf("gain1") >= 0) gain1 = newIntSetting("gain1", inputString);
   if (inputString.indexOf("chanOn0") >= 0) chanOn0 = newIntSetting("chanOn0", inputString);
   if (inputString.indexOf("chanOn1") >= 0) chanOn1 = newIntSetting("chanOn1", inputString);
  }
  adc0_isr();
  adc1_isr();
  if (ledCount > 0) ++ledCount;
  if (ledCount == ledCountMax)
  {
    ledCount = 0;
    digitalWrite(LED, HIGH);
  }
}
void adc0_isr(void) 
{
  value0A[ipt0p] = (int) adc->analogReadContinuous(ADC_0); 
  value0 = value0A[ipt0p] - value0A[ipt0m];  
  value0 = multBy2n(value0, gain0);
  if (sign0 < 0) value0 = -value0;
  if (value0 >  maxAcValue0) value0 =  maxAcValue0;
  if (value0 < -maxAcValue0) value0 = -maxAcValue0;
  if (chanOn0 < 1) value0 = 0;
  value0 = value0 + dacOffset0;
  analogWrite(A21, value0);
 
  ipt0p = ipt0p + 1;
  if (ipt0p == NMAX0) ipt0p = 0;
  ipt0m = ipt0p - timeOffset0;
  if (ipt0m < 0) ipt0m = NMAX0 + ipt0m;
  
}
void adc1_isr(void) 
{
  value1A[ipt1p] = (int) adc->analogReadContinuous(ADC_1); 
  value1 = value1A[ipt1p] - value1A[ipt1m];  
  value1 = multBy2n(value1, gain1);
  if (sign1 < 0) value1 = -value1;
  if (value1 >  maxAcValue1) value1 =  maxAcValue1;
  if (value1 < -maxAcValue1) value1 = -maxAcValue1;
  if (chanOn1 < 1) value1 = 0;
  value1 = value1 + dacOffset1;
  analogWrite(A22, value1);
 
  ipt1p = ipt1p + 1;
  if (ipt1p == NMAX1) ipt1p = 0;
  ipt1m = ipt1p - timeOffset1;
  if (ipt1m < 0) ipt1m = NMAX1 + ipt1m;
  

}
int multBy2n(int value, int n)
{
  boolean sign = false;
  int copyValue = value;
  if (value < 0)
  {
    sign = true;
    copyValue = -copyValue;
  }
  if (n >= 0)
  {
    copyValue = copyValue << n;
  }
  else
  {
    copyValue = copyValue >> -n;
  }
  if (sign) copyValue = -copyValue;
  return copyValue;  
}
int newIntSetting(String setParse, String input)
{
   newSettingString = input.substring(input.lastIndexOf(setParse) + setParse.length() + 1,input.length());
   newSettingString.toCharArray(intbuf, sizeof(intbuf));
   return atoi(intbuf);
}
void readSerial()
{
  inputString = "";
  while(Serial.available() > 0)
  {
    if (!prevSerialAvailableState) 
    {
      digitalWrite(LED, LOW);
      prevSerialAvailableState = true;
    }
    lastRecvd = Serial.read();
    inputString += lastRecvd;
  }
  if (prevSerialAvailableState) 
  {
    prevSerialAvailableState = false;
    ledCount = 1;
  }

}

