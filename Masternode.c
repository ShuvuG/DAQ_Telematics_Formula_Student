//Master controller code 

//Last modified - 29/04/2020
//Shuvechchha Ghimire 
//Version 4
//Target - ATMega328P microcontroller board

//Design considerations: ISR or polling?
// Masks and Filters design
//Communication with the GUI and implementation of SocketCAN - Raspberry Pi
//Store received data in CSV file in SD card 
//Store node lookup table
//Send NMT message - start, stop, reset, configure
//Send error control message - if missing heartbeat message 
//Send time synchronising message
//Heart beat message received - 4 
//Sensor data received -8
//Both have different lengths

#include <SPI.h> 
#include <mcp_can.h> 
#include <time.h> 
#include <SD.h>
/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB #else
#define SERIAL Serial #endif
//Need to find out what assigns this to the SD card storage functionality //decide later after thinking through about the sensor
const int SPI_CS_SD = 4;
//name of the files File Node1;
File Node2;
File CAN_file;
//Lookup table for the node information
//This data is stored in a CSV file after initial configuration struct Lookup_NODE
{
  unsigned int misc1; //to keep track of node state: intialises, started, stopped, restarted unsigned int NODE_Id;
  unsigned int Serial_baud;
  unsigned int Network_baud;
  unsigned int heartbeat_time;
  unsigned int error_repeat;
  unsigned char interruptPin; //Digital pin corresponding to interrupt pin (2 or 3)
  unsigned char CS_PIN;
  unsigned char Status;
  unsigned char Op_mode;
  unsigned long Sample_period; //Sampling period of the node: standard bus scheduling unsigned char Filter; 
  //Use existing Kalman Filter: Yes(1) or No(0)
  //Chip Select pin (9 or 10)
  //Active(1) or standby(2) as it relates to SRAM and EEPROM
  //using interrupt service routines (1) or polling mode(2)
  unsigned char Comms_mode; //Hard and real soft time events: Yes(1) or No(0)
  unsigned long global_timer; //For synchronisation/timestamp calibration purposes : experimentally verify timing for all
};
struct Masks_Filters{
  volatile unsigned long mask1; 
  volatile unsigned long mask2; 
  volatile unsigned long filter1; 
  volatile unsigned long filter2; 
  volatile unsigned long filter3; 
  volatile unsigned long filter4; 
  volatile unsigned long filter5; 
  volatile unsigned long filter6;
};

struct Configuration {
  unsigned char function_code; 
  unsigned int Serial_baud; 
  unsigned int Network_baud; 
  unsigned int NODE_ID; 
  unsigned int heartbeat_time; 
  unsigned int error_repeat; 
  unsigned int CS_pin; 
  unsigned int interruptPin;
};

struct CAN_type{
  volatile unsigned int interruptPin; 
  volatile unsigned int CS_PIN;
};
//--------------------------------------------------------------
// AUTOSETUP -DEFUALT PARAMETERS 
//--------------------------------------------------------------
//15 is the network baud rate based on the CAN controller library
//Need to decide about the node initialisation from Windows form Visual Studio
Lookup_NODE Master = {0, 1, 9600, 15, 1000, 250, 2, 10, 1, 2, 10, 0, 0, 0}; //On how to synchronise all 
Lookup_NODE A = {0, 4, 9600, 15, 1000, 250, 2, 10, 1, 2, 10, 0, 0, 0};
Lookup_NODE B = {0, 120, 115200, 16, 1000, 250, 2, 10, 1, 2, 10, 0, 0, 0}; 
Masks_Filters Parameter2 = {1023, 1023, 0, 0, 0, 0, 0, 0}; //Masks and Filters 
CAN_type Shield = {2,10};
//Write the parameters after configuration - when it communicates with Windows Forms
void setup() 
{
//Set SPI connection using Chip Select pin 
  int SPI_CS_PIN = Shield.CS_PIN; 
  MCP_CAN CAN(SPI_CS_PIN);
  Serial.begin(Master.Serial_baud); 
  if(!SD.begin(5)){

    Serial.println("SD initialisation failed");

    while(1); //infinite loop until external hardware restart/code changes
  }
  Serial.println("SD initialisation done");

  fileWrite(); //Writes the intialisation parameter of all

  while (CAN_OK != CAN.begin(Master.Network_baud)) 
  {
    SERIAL.println("CAN BUS Shield init fail"); 
    SERIAL.println(" Init CAN BUS Shield again"); 
    delay(100);
  }
  SERIAL.println("CAN BUS Shield init ok!");
  // init can bus : baudrate = 500k

  attachInterrupt(digitalPinToInterrupt(Master.interruptPin), MCP2515_ISR, FALLING);
  //send node calibration parameter - function code = 0
  Configuration Slave1 = {0, A.Serial_baud, A.Network_baud, A.NODE_Id, A.heartbeat_time,
  A.error_repeat, A.CS_PIN, A.interruptPin};
  Configuration Slave2 = {0, B.Serial_baud, B.Network_baud, B.NODE_Id, B.heartbeat_time,
  B.error_repeat, B.CS_PIN, B.interruptPin}; 
  //CAN.sendMsgBuf (0x1, 0, 8, Slave1);
  //CAN.sendMsgBuf (0x1, 0, 8, Slave2);
  //synchronisation();
  // start interrupt
}
  
//------------------------------------------------------------
// SEND CAN MESSAGES
//------------------------------------------------------------
//Function to send CAN message and serially print out what was send
void CAN_send(unsigned long CAN_id, unsigned char data_length, unsigned char CAN_message[]) 
{
  CAN.sendMsgBuf(CAN_id, 0, data_length, CAN_message); //CAN_ID, type of message - standard or extended, data length and data
  //Uncomment this in overall implementation 
    /*
    Serial.println("Printing the sent CAN message"); 
    for (int i=0; i<data_length; i++)
  {
  Serial.print(CAN_message[i]);
  Serial.print("\t"); }
  Serial.println(); //new line after printing the sent CAN message
  */
}
  
//Simplistic program that asks user for parameters specified in the GUI through Arduino Serial Monitor
/------------------------------------------------------------
// READ CAN MESSAGES 
//------------------------------------------------------------ 
char CAN_read()
{
  unsigned char len;
  unsigned long CAN_ID; //CAN_ID retreival using library function call 
  unsigned char CAN_message[8]; //Received CAN message
  CAN.readMsgBuf(&len, CAN_message); // read data, len: data length, buf: data buf 
  CAN_ID = CAN.getCanId(); //get CAN id
  if (CAN_ID == 0 and len ==8) //checks whether NMT message is received from remote host
  {
    if(CAN_message[0]==1 or CAN_message[0] ==2) //checks whether CAN stop/start 
    {
      return CAN_message[0]; //returns function code 
    }
      else if(CAN_message[0] ==3) //checks whether NMT reset/configurations 
    {
      configurate(CAN_message);
      return 0; //returns 0 
    }
    else if (CAN_message[0] == 5) 
    {
      //based on NMT reset or initialisation
      view_historic(); //request to view historic data
      return 0; 
    }
  }
  else if (CAN_ID == 0 and len ==8) 
  {
    CAN_receive(CAN_message);
    return 0; 
  }
  else if (len ==4) 
  {
    Serial.println("Error control"); 
    error_control(CAN_message); 
    return 0;
  } 
}
  
//checks whether data received from slaves //record CAN data in SD card or print on screen
//error control subroutine
//----------------------------------------------
// ERROR CONTROL 
//--------------------------------------------------
 //prompted when 4 byte long error frame is received 
 void error_control(unsigned char CAN_message[]) 
 {
  
  unsigned int timer1, timer2; // to idenitfy the producer time 
   Serial.println("Starting error control protocol");
  if (CAN_message[1] ==1) //invalid node
  {
    Serial.println("Invalid node error received from:"+ CAN_message[3]);
    NMT_reset(CAN_message[3]); 
  }
  else if (CAN_message[1]==245) //synchronisation message 
  {
    Serial.println("Synchronisation error");
    NMT_reset(CAN_message[3]); 
  }
  else if (CAN_message[1]==11) //heartbeat message received 
  {
  Serial.println("Heartbeat signal detected"); 
  }
  Serial.println("Exiting error control"); 
}

//-------------------------------------------------
// DATA RECEIVE AND STORE IN SD CARD 
//-------------------------------------------------
//Stores all raw CAN data in the CSV file, if not being overwritten on 
void CAN_receive( unsigned char CAN_message[])
{
  unsigned long canId = CAN.getCanId(); 
  timer = millis();
  myFile.print(timer);
  SERIAL.println("-----------------------------"); 
  SERIAL.print("Get data from ID: 0x"); 
  SERIAL.println(canId, HEX); myFile.print(id);
  myFile.print(","); 
  for(int i = 0; i<len; i++) 
  {
    SERIAL.print(buf[i], HEX); 
    SERIAL.print("\t"); 
    myFile.print(buf[i]); 
    myFile.print(",");
  } 
  SERIAL.println(); 
  myFile.println();
}
//Write node configuration parameters in a seperate CSV file 
//Currently only writes data for two nodes
void fileWrite()
{
  Node1 = SD.open("NODE1.csv", FILE_WRITE); 
  Node1.println("NODE A configuration parameters:"); 
  for (int i = 0; i< sizeof(A); i++)
  {
    Node1.println(buf[i]); 
  }
  Node1.close();
  Node2 = SD.open("NODE2.csv", FILE_WRITE); 
  Node2.println("NODE B configuration parameters:"); 
  for (int i = 0; i<sizeof(B); i++)
  {
    Node2.println(buf[i]); }
    Node2.close();
    Master1 = SD.open("Master.csv", FILE_WRITE); 
  Node2.println("Master configuration parameters:"); 
  for (int i = 0; i<sizeof(Master); i++)
  {
    Master1.println(buf[i]); 
  }
  Master1.close(); 
}

//------------------------------------------------------------
// NODE CONFIGURATION 
//------------------------------------------------------------
//Still need to think about starting and stopping CAN communication 
void configurate(unsigned char CAN_message[])
{

  Serial.println("Configuring/Restarting the system");
  Parameter1 = {0, CAN_message[1],CAN_message[2], CAN_message[3], CAN_message[4],
  CAN_message[5], 0, 0};
//accepts NMT, synchronisation and error control messages from master
//simple conversion to decimal - 1792+Node_ID

  Parameter2 = {1023, 1023, 0x1, 0x80, 1792+CAN_message[3], 1, 80, 1792+CAN_message[3]}; 
  Shield = {CAN_message[6], CAN_message[7]};
setup();
}

//------------------------------------------------------------ 
// INTERRUPT SERVICE ROUTINES
//------------------------------------------------------------
//Interrupt service routine changes flagRecv value 
void MCP2515_ISR()
{
flagRecv = 1; }

//------------------------------------------------------------ 
// Start CAN
//------------------------------------------------------------ 
void start_CAN(unsigned char global_timer)
{
//program these functions based on the the CAN start and CAN stop functionalities 
  myFile = SD.open("can.csv", FILE_WRITE);
  myFile.close();
  if (timer > global_timer)
    {
    timer1 = timer;
    Serial.println("Sending heartbeat message"); 
    CAN.sendMsgBuf (0x700, 0, 4, heartbeat);
    } 
}
//send heartbeat message
//need to use time before another reset can be sent
void NMT_reset(unsigned int NODE_ID)
{
  CAN_configuration(1, CAN_message); CAN.sendMsgBuf(0, 0, 8, CAN_message); Serial.print("CAN message sent");
}

  timer1 = millis()-timer2;
  //need to implement some kind of time measurement
  //heartbeat monitoring protocol needs to be worked on as a single entity 
  Heartbeat_monitoring(NODE_ID, timer1, timer2);
  timer2= timer1;
  //functions compared between two consecutive time that the heartbeat message took
  //If heartbeat message took more than twice to reach, node prints on the screen a message to check the node 
  void Heartbeat_monitoring(unsigned int NODE_ID, unsigned int timer1,unsigned int timer2)
  {
  signed int n=0;
  if (timer1> 2*timer2) 
  {
    Serial.println("node may be inactive- check");
    n=n+1; 
  }
  else {
    Serial.println("Active node performing all right");
    n=n-1;
  }
  if (n>10) {
    NMT_reset(NODE_ID); 
  }
}

//--------------------------------------------------------------- 
// END OF FILE
//---------------------------------------------------------------
