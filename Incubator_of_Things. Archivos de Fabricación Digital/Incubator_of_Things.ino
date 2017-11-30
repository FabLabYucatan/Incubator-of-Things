//Código de funcionamiento del equipo IoT = Incubator of Things, desarrollado por el grupo ChuburLab en Fablab Yucatán.
//Última actualización: 30 de Octubre de 2017
//Contacto:
//http://fablabyucatan.org
//https://www.facebook.com/FabLabYucatan/
//"When you learn to code, it opens up for you to learn many other things" - Mitchel Resnick, director del departamento Lifelong Kindregarten del MIT Media Lab. 

//LIBRERIAS
#include <LiquidCrystal_I2C.h> //Pantalla
#include <SoftwareSerial.h> //CO2
#include <DallasTemperature.h>  //Temperatura

//DEFINCIONES
#define ONE_WIRE_BUS 12 //Temperatura
LiquidCrystal_I2C lcd(0x3F, 16, 2); //Pantalla | El tutorial dice 0x27
SoftwareSerial mySerial(10, 11); //CO2 | RX, TX
SoftwareSerial serialWifi(3,4); //Wifi | RX, TX
OneWire oneWire(ONE_WIRE_BUS); //Temperatura
DallasTemperature sensors(&oneWire);  //Temperatura

//VARIABLES GLOBALES
int controlFoco = 13; //Foco
float sensorCo2 = 0.0;  //CO2
float sensorTemp = 0.0; //Temperatura

//VARIABLES DE FUNCIONES
String val = ""; //CO2 | holds the string of the value
float co2 = 0; //CO2 | holds the actual value
int multiplier = 10; //CO2 | each range of sensor has a different value. Up to 2% = 1. Up to 65% = 10. Up to 100% = 100
uint8_t buffer[25]; //CO2
uint8_t ind = 0; //CO2
float temp = 0.0; //Temperatura

void setup() 
{
  //ESP.wdtDisable();
  //ESP.wdtEnable(WDTO_8S);
  Serial.begin(9600);
  sensors.begin();  //Temperatura. De la función de DallasTemperature
  mySerial.begin(9600); //CO2
  pinMode(controlFoco, OUTPUT); //Foco
  Serial.println("Incubator of Things");
  lcd.init();lcd.backlight();lcd.clear();lcd.setCursor(0,0);lcd.print("IoTs | ChuburLab");lcd.setCursor(0,1);lcd.print("Incubator of Things");  //Pantalla
  delay(1000);  //Display
  lcd.clear();  //Display
}

void loop() 
{
//Sensado CO2
  sensorCo2 = CO2Sensor()/10000;  //Convirtiendo ppm en %
  ind = 0; //Reset the buffer index to overwrite the previous packet
  val = ""; //Reset the value string
//Sensado Temperatura
  sensorTemp = TemperatureSensor();
//Control
  if(sensorTemp > 37)
  {
    digitalWrite(controlFoco, LOW);  
  }
  if(sensorTemp < 34)
  {
    digitalWrite(controlFoco, HIGH);  
  }
//Display
  //Serial.println(sensorCo2/10000);
  //Serial.println(sensorTemp) 
  lcd.setCursor(0,0);
  lcd.print("CO2 = ");
  if(sensorCo2 < 100)
  {
    lcd.print(sensorCo2);  
  }
  lcd.setCursor(10,0);
  lcd.print("%    ");
  lcd.setCursor(0,1);
  lcd.print("Temp = ");
  lcd.print(sensorTemp);
  lcd.setCursor(12,1);
  lcd.print(" C");
  
  
}

float CO2Sensor()
{
  //Cycle through the buffer and send out each byte including the final linefeed
  /*
    each packet in the stream looks like "Z 00400 z 00360"
    'Z' lets us know its a co2 reading. the first number is the filtered value
    and the number after the 'z' is the raw value.
    We are really only interested in the filtered value
  */
  //Lectura del C02
  while (buffer[ind - 1] != 0x0A)
  {
    //Serial.println("Sí");
    if (mySerial.available())
    {     
      buffer[ind] = mySerial.read();
      ind++;
    }
    yield();  //Para que no se resetee el ESP1866. https://forum.arduino.cc/index.php?topic=442570.0
  }
  
  for (int i = 0; i < ind + 1; i++)
  {
    if (buffer[i] == 'z') //once we hit the 'z' we can stop
      break;

    if ((buffer[i] != 0x5A) && (buffer[i] != 0x20)) //ignore 'Z' and white space
    {
      val += buffer[i] - 48; //because we break at 'z' the only bytes getting added are the numbers
      // we subtract 48 to get to the actual numerical value
      // example the character '9' has an ASCII value of 57. [57-48=9]
    }
  }

  co2 = (multiplier * val.toInt()); //now we multiply the value by a factor specific ot the sensor. see the Cozir software guide
  //Serial.print( "Co2 = ");//Serial.print(co2/10000);//Serial.println(" ppm");//lcd.setCursor(0,0);//lcd.print(co2/10000);//lcd.print(" %");//delay(1000);//lcd.clear();
  return co2;
}

float TemperatureSensor()
{
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);
  return temp;
}
