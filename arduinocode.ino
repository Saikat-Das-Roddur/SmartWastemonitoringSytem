#include <SoftwareSerial.h>
#include <Servo.h>
#include "HX711.h"
#include "SPI.h"
#include "MFRC522.h"

SoftwareSerial gprsSerial(7, 8);

 HX711 cell(5,6);
#define SS_PIN 10
#define RST_PIN 9
#define SP_PIN 8
Servo servo1;
int sensor=A5;
int gas_value;

int servoPin = 4;
float userWeight;
long val=0;
 float count=0;
 
MFRC522 rfid(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;
long duration, distance, RightSensor,BackSensor,FrontSensor,LeftSensor,finalDistance1,finalDistance2;

void setup()
{
 pinMode(A2, OUTPUT);
 pinMode(A3, INPUT);
 pinMode(A0, OUTPUT);
 pinMode(A1, INPUT);
 

pinMode(sensor,INPUT); 
 
  gprsSerial.begin(9600);
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  
  
  Serial.println("Config SIM800...");
  delay(2000);
  Serial.println("Done!...");
  gprsSerial.flush();
  Serial.flush();

  // attach or detach from GPRS service 
  gprsSerial.println("AT+CGATT?");
  delay(100);
  toSerial();


  // bearer settings
  gprsSerial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(2000);
  toSerial();

  // bearer settings
  gprsSerial.println("AT+SAPBR=3,1,\"APN\",\"gpinternet\"");
  delay(2000);
  toSerial();

  // bearer settings
  gprsSerial.println("AT+SAPBR=1,1");
  delay(2000);
  toSerial();
 

}


void loop()
{
 
   if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  // Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  // Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  String strID = "";
  for (byte i = 0; i < 4; i++) {
    strID +=
    (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
    String(rfid.uid.uidByte[i], HEX) +
    (i!=3 ? ":" : "");
  }
  strID.toUpperCase();

  Serial.print("Tap card key: ");
  int b =strID.toInt() ;
  
   Serial.println(b);

   gprsSerial.println("AT+HTTPINIT");
   delay(2000); 
   toSerial();

   //String str="GET https://api.thingspeak.com/update?api_key="+ api_key +"&field"+ field +"="+ String(distance); //thinkspeak data writing url

  String str1="AT+HTTPPARA=\"URL\",\"http://fullstackweb.000webhostapp.com/waste-management/ID.php?id="+String(b); //thinkspeak data writing url

//  String str="AT+HTTPPARA=\"URL\",\"http://fullstackweb.000webhostapp.com/waste-management/write_data.php?data1=distance";

  gprsSerial.println(str1);

 
   delay(2000);
   toSerial();

   // set http action type 0 = GET, 1 = POST, 2 = HEAD
   gprsSerial.println("AT+HTTPACTION=0");
   delay(6000);
   toSerial();

   // read server response
   gprsSerial.println("AT+HTTPREAD"); 
   delay(1000);
  // toSerial();
   String c = gprsSerial.readString();
   Serial.println(c);
   String userName = c.substring(29,44);
   Serial.println(userName);

   String userMobile  = c.substring(44,55);
   Serial.println(userMobile);

   String userStatus  = c.substring(55,57);
   Serial.println(userStatus);

   String userWeight1 = c.substring(57,58);
   Serial.println(userWeight1);

   String userBalance1 = c.substring(58,60);
   Serial.println(userBalance1);
   
   //sms
  // ServoMotor(userStatus);
  if(userStatus=="ok"){
  servo1.attach(servoPin);
  servo1.write(180);
  delay(10000);
  servo1.write(0);
  delay(10000);
  servo1.detach();
  }
   
  //sent level data
 //weight
 val=.8*val+.2*cell.read();
 userWeight = ((cell.read()-110605)/930.08f);
 delay(1000);
 //Serial.println((cell.read()-110605)/930.08f);
 //weight end
 float userBalance = (userWeight/1000)*8;
  
   gprsSerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
   delay(1000);  // Delay of 1000 milli seconds or 1 second
   gprsSerial.println("AT+CMGS=\""+userMobile+"\"\r");  // Replace x with mobile number
   delay(1000);
   gprsSerial.println("Hello "+userName+",\n Your total weight is: "+userWeight+"gm,\n Your Balance is: "+userBalance+"/=BDT\n" );// The SMS text you want to send
   delay(100);
   gprsSerial.println((char)26);// ASCII code of CTRL+Z
   delay(1000);
    
    
//gas sensor

 gas_value=analogRead(sensor);
delay(2000);
//Serial.println(gas_value);

//gas sensor end
   //ultrasonic sensor
//ServoMotor();
  SonarSensor(A2, A3);
  RightSensor = distance;
  finalDistance1=(36-RightSensor)*2.78;
  SonarSensor(A0, A1);
  LeftSensor = distance;
  finalDistance2=(37-LeftSensor)*2.703;
   // initialize http service
  //  distance2=10;
   gprsSerial.println("AT+HTTPINIT");
   delay(2000); 
   toSerial();

   //String str="GET https://api.thingspeak.com/update?api_key="+ api_key +"&field"+ field +"="+ String(distance); //thinkspeak data writing url

  String str="AT+HTTPPARA=\"URL\",\"http://fullstackweb.000webhostapp.com/waste-management/write_data.php?data1="+String(finalDistance1)+"&data2="+String(finalDistance2)+"&gas="+String(gas_value); //thinkspeak data writing url

//  String str="AT+HTTPPARA=\"URL\",\"http://fullstackweb.000webhostapp.com/waste-management/write_data.php?data1=distance";

  gprsSerial.println(str);

 // String str2="AT+HTTPPARA=\"URL\",\"http://fullstackweb.000webhostapp.com/waste-management/write_data.php?data2="+String(finalDistance2);
 // gprsSerial.println(str2);
   // set http param value
  // gprsSerial.println("AT+HTTPPARA=\"URL\",\"http://fullstackweb.000webhostapp.com/write_data.php?data1=String(distance)\"");
   delay(2000);
   toSerial();

   // set http action type 0 = GET, 1 = POST, 2 = HEAD
   gprsSerial.println("AT+HTTPACTION=0");
   delay(6000);
   toSerial();

   // read server response
   gprsSerial.println("AT+HTTPREAD"); 
   delay(1000);
   toSerial();

   gprsSerial.println("");
   gprsSerial.println("AT+HTTPTERM");
   toSerial();
   delay(300);

   gprsSerial.println("");
   delay(10000);
   //ultrasonic sensor end
   //sms
   gprsSerial.println("");
   gprsSerial.println("AT+HTTPTERM");
   toSerial();
   delay(300);

   gprsSerial.println("");
   delay(10000);

   rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  
   gprsSerial.println("");
   delay(10000);
}


void toSerial()
{
  while(gprsSerial.available()!=0)
  {
    Serial.write(gprsSerial.read());
  }
}
void SonarSensor(int trigPin,int echoPin)
{
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
duration = pulseIn(echoPin, HIGH);
distance= duration*0.034/2;
 
}

//void ServoMotor(userStatus){
  
//}

