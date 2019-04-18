#include "define.h"
#include "stdio.h"
#include "stdlib.h"  
#include "wiringPi.h"
#include "wiringSerial.h"
#include <string.h>

int var; //for raspberry UART handle

FILE *pFile;

void sendCommand(CHAR *Data,INT length)
{
   INT i;
   for(i = 0; i < length; i++)
      serialPutchar(var,*(Data + i));
}

void receiveCommand(CHAR *Data,INT length)
{
   INT i=0,time_out=0;
   
   do
    {
     if(serialDataAvail(var)>0)  //check RX buffer
       {
       if(i < length)
       	{
         *(Data + i) = serialGetchar(var);
         i++;         
        }
       }
     else
       {
       delay(10);
       time_out++;
       if(time_out==300)
         {
          printf("No Fingerprint Module Detected\n");
          exit(0);  
         }
       }
    }while(i<length);  //check total package length
}

SHORT CalcChkSumOfCmdAckPkt(COMMAND_PACKET *pPkt)
{
   SHORT checkSum = 0;
   CHAR *pBuf = (CHAR*)pPkt;
   int i;
	
   for(i=0;i<(sizeof(COMMAND_PACKET)-2);i++)
      checkSum += pBuf[i];

   return checkSum;
}

SHORT CalcChkSumOfDataPkt(DATA_PACKET *pPkt)
{
   SHORT checkSum = 0;
   CHAR *pBuf = (CHAR*)pPkt;
   int i;
	
   for(i=0;i<(sizeof(DATA_PACKET)-2);i++)
      checkSum += pBuf[i];

   return checkSum;
}

void send_receive_command()
{
   sendCommand(&commandPacket.start1,COMMAND_PACKAGE_LENGTH);
   receiveCommand(&commandPacket.start1,COMMAND_PACKAGE_LENGTH);  
  
   returnParameter=commandPacket.parameter;
   returnAck=commandPacket.command;
}

void Open()
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=0x00000000;
  commandPacket.command=OPEN;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command();
}

void Close()
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=0x00000000;
  commandPacket.command=CLOSE;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command();
}

void LED_open()
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=0x00000001; 
  commandPacket.command=CMOSLED;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command();
}

void LED_close()
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=0x00000000; 
  commandPacket.command=CMOSLED;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command();
}

void EnrollStart(int specify_ID)
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=specify_ID; 
  commandPacket.command=ENROLLSTART;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command(); 
}  

void EnrollStart1(int specify_ID)
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=specify_ID; 
  commandPacket.command=ENROLLSTART;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command(); 

  
}  

void Enroll1()
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=0x00000000;
  commandPacket.command=ENROLL1;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
   send_receive_command();}

void Enroll2()
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=0x00000000;
  commandPacket.command=ENROLL2;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
   send_receive_command();}

void Enroll3()
{ 
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=0x00000000;
  commandPacket.command=ENROLL3;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
   send_receive_command();

  if(returnAck!=ACK)
		{delay(500);
		 printf("Enrollment Could Not Be Completed\n");
	   return;
	  }
    delay(500);
    printf("Take off finger\n");
    if (returnAck == ACK) //decide whether enroll ok or fail
    
        {//delay(500);
          printf("Enroll Finish!\n");
        receiveCommand(&dataPacket.start1, DATA_PACKAGE_LENGTH); //read template to receive buffer from fingeprint module
     
    char filename[64];  

    sprintf(filename, "./%s.bin", "tpl");
    pFile = fopen(filename, "w");
    if (NULL == pFile)
    {delay(500);
      printf("Open failure");
      return;
    }
    else
      fwrite(dataPacket.data, 1, sizeof(dataPacket.data), pFile);

    fclose(pFile);
    delay(500);
    printf("Created %s on this folder!\n", filename);
        }
        else
        {
          if (returnParameter >= 0 && returnParameter <= 199)
            //ay(500);
            printf("Enroll Failed: Duplicate Finger\n");
          else{
            printf("Enroll Failed! Try Again\n");}
        }
  
   
}

void Identify()
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=0x00000000; 
  commandPacket.command=IDENTIFY;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command();
}

void DeleteId(int specify_ID)
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=specify_ID; 
  commandPacket.command=DELETEID;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command(); 
}  

void Enroll(int Enroll_define)
{
  SHORT Enroll_command;
  
  switch(Enroll_define)
    {
      case 1:
             Enroll_command=ENROLL1;
             break;
      case 2:
             Enroll_command=ENROLL2;
             break;
      case 3:
             Enroll_command=ENROLL3;
             break;      
    }
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=0x00000000;
  commandPacket.command=Enroll_command;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command(); 
}

void IsPressFinger()
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=0x00000000; 
  commandPacket.command=ISPRESSFINGER;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command();  
} 

void CaptureFinger(LONG picture_quality)
{ 
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=picture_quality; 
  commandPacket.command=CAPTURE_FINGER;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command();  
}

void DeleteAll()
{
  commandPacket.start1=COMMAND_START_CODE1;  
  commandPacket.start2=COMMAND_START_CODE2;
  commandPacket.deviceId=DEVICE_ID;
  commandPacket.parameter=0x00000000; 
  commandPacket.command=DELETEALL;
  commandPacket.checkSum=CalcChkSumOfCmdAckPkt(&commandPacket);
  
  send_receive_command();
}