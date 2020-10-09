/******************************************************************************
  BME280Compensated.ino
  Marshall Taylor @ SparkFun Electronics
  April 4, 2017
  https://github.com/sparkfun/CCS811_Air_Quality_Breakout
  https://github.com/sparkfun/SparkFun_CCS811_Arduino_Library
  This example uses a BME280 to gather environmental data that is then used
  to compensate the CCS811.
  Hardware Connections (Breakoutboard to Arduino):
  3.3V to 3.3V pin
  GND to GND pin
  SDA to A4
  SCL to A5
  Resources:
  Uses Wire.h for i2c operation
  Development environment specifics:
  Arduino IDE 1.8.1
  This code is released under the [MIT License](http://opensource.org/licenses/MIT).
  Please review the LICENSE.md file included with this example. If you have any questions
  or concerns with licensing, please contact techsupport@sparkfun.com.
  Distributed as-is; no warranty is given.
******************************************************************************/
#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>

#define CCS811_ADDR 0x5B //Default I2C Address
//#define CCS811_ADDR 0x5A //Alternate I2C Address

//Global sensor objects
CCS811 myCCS811(CCS811_ADDR);
BME280 myBME280;

void setup()
{
  SerialUSB.begin(9600);
  SerialUSB.println();
  SerialUSB.println("Apply BME280 data to CCS811 for compensation.");
  
  Wire.begin();//initialize I2C bus
  
  //This begins the CCS811 sensor and prints error status of .begin()
  CCS811Core::CCS811_Status_e returnCode = myCCS811.beginWithStatus ();
  if (returnCode != CCS811Core::CCS811_Stat_SUCCESS)
  {
    SerialUSB.println("Problem with CCS811");
    printDriverError(returnCode);
  }
  else
  {
    SerialUSB.println("CCS811 online");
  }

  
  //Initialize BME280
  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x77;
  myBME280.settings.runMode = 3; //Normal mode
  myBME280.settings.tStandby = 0;
  myBME280.settings.filter = 4;
  myBME280.settings.tempOverSample = 5;
  myBME280.settings.pressOverSample = 5;
  myBME280.settings.humidOverSample = 5;

  //Calling .begin() causes the settings to be loaded
  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  byte id = myBME280.begin(); //Returns ID of 0x60 if successful
  if (id != 0x60)
  {
    SerialUSB.println("Problem with BME280");
  }
  else
  {
    SerialUSB.println("BME280 online");
  }


}
//---------------------------------------------------------------
void loop()
{
  //Check to see if data is available
  if (myCCS811.dataAvailable())
  {
    //Calling this function updates the global tVOC and eCO2 variables
    myCCS811.readAlgorithmResults();
    //printData fetches the values of tVOC and eCO2
    printData();

    float BMEtempC = myBME280.readTempC();
    float BMEhumid = myBME280.readFloatHumidity();

    SerialUSB.print("Applying new values (deg C, %): ");
    SerialUSB.print(BMEtempC);
    SerialUSB.print(",");
    SerialUSB.println(BMEhumid);
    SerialUSB.println();

    //This sends the temperature data to the CCS811
    myCCS811.setEnvironmentalData(BMEhumid, BMEtempC);
  }
  else if (myCCS811.checkForStatusError())
  {
    SerialUSB.println(myCCS811.getErrorRegister()); //Prints whatever CSS811 error flags are detected
  }

  delay(2000); //Wait for next reading
}

//---------------------------------------------------------------
void printData()
{
  SerialUSB.print(" CO2[");
  SerialUSB.print(myCCS811.getCO2());
  SerialUSB.print("]ppm");

  SerialUSB.print(" TVOC[");
  SerialUSB.print(myCCS811.getTVOC());
  SerialUSB.print("]ppb");

  SerialUSB.print(" temp[");
  SerialUSB.print(myBME280.readTempC(), 1);
  SerialUSB.print("]C");

  //SerialUSB.print(" temp[");
  //SerialUSB.print(myBME280.readTempF(), 1);
  //SerialUSB.print("]F");

  SerialUSB.print(" pressure[");
  SerialUSB.print(myBME280.readFloatPressure(), 2);
  SerialUSB.print("]Pa");

  //SerialUSB.print(" pressure[");
  //SerialUSB.print((myBME280.readFloatPressure() * 0.0002953), 2);
  //SerialUSB.print("]InHg");

  //SerialUSB.print("altitude[");
  //SerialUSB.print(myBME280.readFloatAltitudeMeters(), 2);
  //SerialUSB.print("]m");

  //SerialUSB.print("altitude[");
  //SerialUSB.print(myBME280.readFloatAltitudeFeet(), 2);
  //SerialUSB.print("]ft");

  SerialUSB.print(" humidity[");
  SerialUSB.print(myBME280.readFloatHumidity(), 0);
  SerialUSB.print("]%");

  SerialUSB.println();
}

//printDriverError decodes the CCS811Core::status type and prints the
//type of error to the SerialUSB terminal.
//
//Save the return value of any function of type CCS811Core::status, then pass
//to this function to see what the output was.
void printDriverError( CCS811Core::CCS811_Status_e errorCode )
{
  switch ( errorCode )
  {
    case CCS811Core::CCS811_Stat_SUCCESS:
      SerialUSB.print("SUCCESS");
      break;
    case CCS811Core::CCS811_Stat_ID_ERROR:
      SerialUSB.print("ID_ERROR");
      break;
    case CCS811Core::CCS811_Stat_I2C_ERROR:
      SerialUSB.print("I2C_ERROR");
      break;
    case CCS811Core::CCS811_Stat_INTERNAL_ERROR:
      SerialUSB.print("INTERNAL_ERROR");
      break;
    //case CCS811Core::SENSOR_GENERIC_ERROR:
    case CCS811Core::CCS811_Stat_GENERIC_ERROR:
      SerialUSB.print("GENERIC_ERROR");
      break;
    default:
      SerialUSB.print("Unspecified error.");
  }
}
