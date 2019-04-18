#include "command.h"

#define COMMAND_PACKAGE_LENGTH 12    //command package area

#define COMMAND_START_CODE1    0x55
#define COMMAND_START_CODE2    0xAA

#define DATA_START_CODE1       0x5A
#define DATA_START_CODE2       0xA5

#define DATA_PACKAGE_LENGTH    498   //data package area

#define DEVICE_ID	           0x0001

#define OPEN		           0x01  //command define
#define CLOSE				   0x02
#define CMOSLED		           0x12
#define GETENROLLCOUNT	       0x20
#define CHECKENROLLED		   0x21
#define	ENROLLSTART	           0x22
#define ENROLL1		           0x23
#define ENROLL2		           0x24
#define ENROLL3		           0x25
#define ISPRESSFINGER	       0x26
#define DELETEID	           0x40
#define DELETEALL	           0x41
#define IDENTIFY	           0x51
#define CAPTURE_FINGER	       0x60
#define GETTEMPLATE            0x70
#define SETTEMPLATE            0x71
#define VERIFYTEMPLATE		   0x52
#define IDENTIFYTEMPLATE	   0x53
#define MAKETEMPLATE           0x61
#define ACK		               0x30
#define NACK		           0x31

typedef struct {		
	CHAR start1;		     
	CHAR start2;		     
	SHORT deviceId;        
	LONG parameter;          
	SHORT command;          
	SHORT checkSum;       
} COMMAND_PACKET;

typedef struct {		
	CHAR start1;		     
	CHAR start2;		     
	SHORT deviceId;        
	CHAR	data [498];   
	SHORT checkSum;
} DATA_PACKET;

extern int var;
LONG returnParameter;    
SHORT returnAck;    

COMMAND_PACKET commandPacket; 
DATA_PACKET dataPacket;