//Slave protocol
//Simplified program with no in-node error detection mechanism

// Shuvechchha Ghimire //Version - 5
//Last modified: 29/04/2020

//Remaining:
//Implement Sensor
//ISR for hard time and soft time
//Time synchronisation
//Lookup tables if calibrating sensor attached to it

#include <SPI.h>
#include <mcp_can.h> 
#include <time.h>
#include <EEPROM.h> 
#include <DataSeriesPod.h>
#include <HCSR04.h>

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB #else
#define SERIAL Serial #endif
//----------------------------------------------------------- // VARIABLE DECLARATIONS //----------------------------------------------------------- //Interrupt Service Routine
static unsigned char flagRecv=0;
static unsigned int function_code =1; //send CAN message when no CAN message is received unsigned char sensor_Threshold =0;
//Variables and arrays to store CAN data
unsigned char EEPROM_storage[8]; //EEPROM storage
//DataSeriesPod library specific variables
DataSeriesPod dsp = DataSeriesPod("Sensor_1");
unsigned long timeInterval = 5UL; // 5ms time interval to fetch statistical parameters
//Configuration parameters are defined in struct formats to ease reconfiguration later struct Configuration
{
unsigned int misc1;
unsigned long Serial_baud; unsigned long Network_baud; unsigned int NODE_ID; unsigned int heartbeat_time; unsigned int error_repeat; unsigned int global_timer; unsigned int misc3;
};
struct Masks_Filters{ unsigned long mask1; unsigned long mask2; unsigned int filter1; unsigned int filter2; unsigned int filter3; unsigned int filter4;
unsigned int filter5;
unsigned int filter6; };
struct CAN_type{
volatile unsigned int interruptPin; volatile unsigned int CS_PIN;
};
//-------------------------------------------------------------
// AUTOSETUP -DEFUALT PARAMETERS //--------------------------------------------------------------
static unsigned char NODE_ID =102; //default node id is 102
Configuration Parameter1 = {0, 115200, 16, NODE_ID, 100, 100, 0, 0};
Masks_Filters Parameter2 = {1023, 1023, 1, 1, 1, 1, 1, 1}; //waiting only for the NMT message CAN_type Shield = {2, 10}; //default using TJa1050 CAN shield
//Set SPI connection using Chip Select pin int SPI_CS_PIN = Shield.CS_PIN; MCP_CAN CAN(SPI_CS_PIN);
//----------------------------------------------------------- // PROGRAM SET UP //----------------------------------------------------------- void setup()
{
//Set SPI connection using Chip Select pin
int SPI_CS_PIN = Shield.CS_PIN; MCP_CAN CAN(SPI_CS_PIN);
Serial.begin(Parameter1.Serial_baud);
while (CAN_OK != CAN.begin(Parameter1.Network_baud)) {
Serial.println("CAN BUS Shield init fail"); Serial.println(" Init CAN BUS Shield again"); delay(50);
}
Serial.println("CAN BUS Shield init ok!");
// init can bus : baudrate = 500k
attachInterrupt(digitalPinToInterrupt(Shield.interruptPin), MCP2515_ISR, FALLING);
CAN.init_Mask(0, 0, Parameter2.mask1); CAN.init_Mask(1, 0, Parameter2.mask2); CAN.init_Filt(0, 0, Parameter2.filter1); CAN.init_Filt(1, 0, Parameter2.filter2); CAN.init_Filt(2, 0, Parameter2.filter3); CAN.init_Filt(3, 0, Parameter2.filter4); CAN.init_Filt(4, 0, Parameter2.filter5); CAN.init_Filt(5, 0, Parameter2.filter6);
}
//------------------------------------------------------------ // INTERRUPT SERVICE ROUTINES //------------------------------------------------------------
//Interrupt service routine changes flagRecv value void MCP2515_ISR()
{
flagRecv = 1; }
//masks //filter
//-------------------------------------------------------------

// MAIN LOOP
//Need to define PArameter1.heartbeat_time //------------------------------------------------------------- void loop()
{
if (flagRecv ==1) //Only executed when new CAN message is received {
function_code = CAN_read(); //Checks if function code has changed
flagRecv = 0; //only one iteration unless changed by MCP2515_ISR }
//Data received is checked based on polling
if (function_code == 1) //start CAN communication {
start_CAN(NODE_ID, Parameter1.heartbeat_time); }
//send data, heartbeat message
else if (function_code ==2) //stop CAN communication {
Serial.println("CAN communication stopped by Master");
eeprom_write(NODE_ID); //if not communicating in CAN bus store data in EEPROM }
else {
Serial.println("In loop"); //Normal CAN communication }
}
//------------------------------------------------------------
// SEND CAN MESSAGES
//------------------------------------------------------------
//Function to send CAN message and serially print out what was send
void CAN_send(unsigned long CAN_id, unsigned char data_length, unsigned char CAN_message[]) {
CAN.sendMsgBuf(CAN_id, 0, data_length, CAN_message); //CAN_ID, type of message - standard or extended, data length and data
//Uncomment this in overall implementation /* Serial.println("Printing the sent CAN message"); for (int i=0; i<data_length; i++)
{
Serial.print(CAN_message[i]);
Serial.print("\t"); }
Serial.println(); // */
}
//print message in Arduino serial monitor
//new line after printing the sent CAN message
//------------------------------------------------------------
// READ CAN MESSAGES into the arduino serial monitor //------------------------------------------------------------
char CAN_read()
{
unsigned char len;
unsigned long CAN_ID; //CAN_ID retreival using library function call unsigned char CAN_message[8]; //Received CAN message
CAN.readMsgBuf(&len, CAN_message); // read data, len: data length, buf: data buf CAN_ID = CAN.getCanId(); //get CAN id
//Uncomment this in overall Arduino Serial monitor implementation (press enter) /*
//This introduces further inconsistency in the program - in the real-time implementation, this would be disabled to facilitate continous message transmission
//For current purposes, decrease the CAN baud rate
Serial.println("CAN message received"); SERIAL.println("\r\n------------------------------------------------------------------");

SERIAL.print("Get Data From id: ");
SERIAL.println(CAN_ID);
for(int i = 0; i<len; i++) // print the data {
SERIAL.print("0x"); SERIAL.print(CAN_message[i], HEX); SERIAL.print("\t");
}
// (press enter) */
if (CAN_ID == 1 and len ==8) //checks whether NMT message is received {
if(CAN_message[0]==1 or CAN_message[0] ==2) //checks whether CAN stop/start {
return CAN_message[0]; //returns function code }
else if(CAN_message[0]==0 or CAN_message[0] ==3) //checks whether NMT reset/configurations {
configurate(CAN_message);
return 0; //returns 0 }
}
//else if (len ==6)
//{
// time_synchronise(CAN_message); // return 0;
//}
else if (len ==4) {
//based on NMT reset or initialisation
//protocol for sync later in the program
Serial.println("Error control");
//unsigned char heartbeat[4] = {0, 11, 0 , NODE_Id};
unsigned char heartbeat[4] = {0, 11, 0 , 0};
CAN.sendMsgBuf (0x700, 0, 4, heartbeat); //sends heartbeat message again return 0;
} }
//------------------------------------------------------------
// NODE CONFIGURATION //------------------------------------------------------------
//Still need to think about starting and stopping CAN communication void configurate(unsigned char CAN_message[])
{
Serial.println("Configuring/Restarting the system");
Parameter1 = {0, CAN_message[1],CAN_message[2], CAN_message[3], CAN_message[4],
CAN_message[5], 0, 0};
//accepts NMT, synchronisation and error control messages from master
//simple conversion to decimal - 1792+Node_ID
Parameter2 = {1023, 1023, 0x1, 0x80, 1792+CAN_message[3], 1, 80, 1792+CAN_message[3]}; Shield = {CAN_message[6], CAN_message[7]};
setup();
}
//------------------------------------------------------------
// Start CAN
//------------------------------------------------------------
void start_CAN(unsigned char NODE_Id, unsigned char global_timer) {
unsigned char SensorData[8]; //Sending CAN message //heartbeat is for monitoring purpose only, hence is static unsigned char heartbeat[4] = {0, 11, 0 , NODE_Id};
//One time declaration of variables for time synchronization

static unsigned int timer1=0; static unsigned int timer =0;
timer= millis()-timer1;
//millis doesnt count for the ISR, ISR is made sufficiently small if (timer > global_timer)
{
timer1 = timer;
Serial.println("Sending heartbeat message");
CAN.sendMsgBuf (0x700, 0, 4, heartbeat); //send heartbeat message
}
get_Sensordata(NODE_Id, SensorData); //get sensor data from SRAM CAN_send(180, 8, SensorData); //Send CAN messages to master
}
//---------------------------------------------------------- // EEPROM write //---------------------------------------------------------- void eeprom_write(unsigned char Node_ID)
{
int addr = 0;
unsigned char sensor_data[8];
get_Sensordata(Node_ID, sensor_data);
for (int i =0; i<8; i++)
{
EEPROM.write(addr, sensor_data[i]);
addr = addr + 1; //EEPROM uses indirect addressing modes if (addr == EEPROM.length()) {
addr = 0; }
} }
//--------------------------------------------------------------
// FETCH DATA FROM SENSORS AND STORE IN SRAM //--------------------------------------------------------------
//The program converts all float/int values to char - this introduces jitter in the program void get_Sensordata(unsigned char node_id, unsigned char sensor_data[])
{
dsp.update(getSensorData()); // This function is called continuosly until the if-loop condition is true
if (dsp.getDuration() >= timeInterval) // check for break condition {
Serial.println("Using buffers to record CAN message"); called and data is recorded for the specified time interval
//Prints on the screen when this function is
sensor_data[0]= node_id;
sensor_data[1] =(char)dsp.getRange();
sensor_data[2] =(char)dsp.getSampleSize();
sensor_data[3] =(char)dsp.getAverageValue();
sensor_data[4] =(char)dsp.getMinimumValue();
sensor_data[5] =(char)dsp.getMaximumValue();
sensor_data[6] =(char)dsp.getFirstValue();
sensor_data[7] =(char)dsp.getDuration();
dsp.restart(); //After consuming the parameters, a restart() is required
} }
//Function to get random data for CAN_MESSAGE for simulation purposes double getSensorData()
{
Serial.println("Getting sensor data"); delay(random(1,10));
double random1 = random(0,1023); //creates gaussian noise

double noise1 = random(1,1023)/1023.0;
double noise2 = random(1,1023)/1023.0;
double random2= sqrt(-2*log(noise1))*cos(2*M_PI*noise2); return (random1+random2);
}
//--------------------------------------------------------------- // END OF FILE //---------------------------------------------------------------
