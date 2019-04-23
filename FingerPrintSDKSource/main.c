#include "stdio.h"
#include "stdlib.h"
#include "define.h"
#include "command.h"
#include <string.h>
#include "wiringPi.h"     //load WiringPi library
#include "wiringSerial.h" //load WiringPi serial library

//Command Line Usage Block
static void print_usage(const char *pcProgramName)
{
    printf("Usage: %s not run properly\nExamples : %s open\n          %s close\n           %senrol\n          %sisPressfinger\n", pcProgramName);
}

int var; //UART Handle

/*Main Function Block*/
int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *command = argv[1];
    int switchNum = 0;

    if (wiringPiSetup() == -1)
        exit(1); //for wiringPi GPIO congiguration

    //check UART coummunication about WiringPi library,Raspberry and Fingerprint
    if (wiringPiSetup() < 0)
    {
        printf("WiringPi-UART error!");
        return -1;
    }

    // Check UART baudrate between FingerPrint Module & FingerPrint
    if ((var = serialOpen("/dev/ttyS0", 9600)) < 0)
    {
        printf("Raspberry-UART error!");
        return -1;
    }

    //Command Switch Case Instances
    if (strcmp(command, "open") == 0)
    {
        switchNum = 1;
    }
    else if (strcmp(command, "finger") == 0)
    {
        switchNum = 2;
    }
    else if (strcmp(command, "start") == 0)
    {
        switchNum = 3;
    }
    else if (strcmp(command, "enroll") == 0)
    {
        switchNum = 4;
    }
    else if (strcmp(command, "close") == 0)
    {
        switchNum = 5;
    }

    //Case Manipulation
    switch (switchNum)
    {
    //OPEN
    case 1:
        Open();
        if (returnAck == ACK)
        {
            LED_open();
            if (returnAck != ACK)
            {
                LED_close();
                return -1;
            }
            fprintf(stdout, "SUCCESS"); // we need to exit the code here.
            return 0;                   // 0 or - 1 ?
        }
        else
        {
            {
                fprintf(stdout, "FAIL");
                return -1;
            }
        }
        break;
    //FINGER
    case 2:
    {
        LED_open();

        while (1) // while finger is not pressed, keep running the isPressedFinger();never enter into this block if the condition is not met.
        {
            IsPressFinger();
            if (returnParameter != 0x1012)
            {

                fprintf(stdout, "SUCCESS FINGER");
                return -1;
            }
        }

        break;
    }
    //ENROLLSTART
    case 3:
    {
        EnrollStart(-1);
        if (returnAck != ACK && returnParameter == 0x1005) //change another IDs if default ID=0 is occupied
        {
            fprintf(stdout, "ENROLL START ::%d", -1);
            return -1;
        }
        else if (returnAck == ACK)
        {
            fprintf(stdout, "ENROLL START ::%d", -1);
            return -1;
        }
        break;
    }

    //ENROLL
    case 4:
    {
        const char *input = argv[2];

        int instance = 0;
        if (strcmp(input, "1") == 0)
        {
            instance = 41;
        }
        else if (strcmp(input, "2") == 0)
        {
            instance = 42;
        }
        else if (strcmp(input, "3") == 0)
        {
            instance = 43;
        }

        int loop_time = 1;
        while (1)
        {
            CaptureFinger(1);
            if (returnAck == ACK)
            {
                break;
            }

            delay(10);
            if (loop_time == 500) //waiting for time out
            {
                fprintf(stdout, "ENROLL TIMEOUT");
                LED_close();
                return -1;
            }
            loop_time++;
        }

        switch (instance)
        {
        case 41:
            Enroll1();
            if (returnAck != ACK)
            {
                fprintf(stdout, "ENROLL FAILED ##%x", returnParameter);
                LED_close();
            }
            else if ((returnAck == ACK))
            {
                fprintf(stdout, "ENROLL SUCCESS ::%d", instance);
                LED_close();
            }
            LED_open();
            return -1;

        case 42:
            Enroll2();
            if (returnAck != ACK)
            {
                fprintf(stdout, "ENROLL FAILED ##%x", returnParameter);
                LED_close();
            }
            else if ((returnAck == ACK))
            {
                fprintf(stdout, "ENROLL SUCCESS ::%d", instance);
                LED_close();
            }
            LED_open();
            return -1;

        case 43:
            Enroll3();
            if (returnAck != ACK)
            {
                fprintf(stdout, "ENROLL FAILED ##%x", returnParameter);
            }
            else if ((returnAck == ACK))
            {
                fprintf(stdout, "ENROLL SUCCESS ::%d", instance);
                LED_close();
            }
            LED_open();
            return -1;
        }
    }

    //CLOSE
    case 5:

        Close();
        if (returnAck == ACK)
        {
            LED_close();
            if (returnAck != ACK)
            {
                LED_close();
                return -1;
            }
            fprintf(stdout, "SUCCESS"); // we need to exit the code here.
            return 0;                   // 0 or - 1 ?
        }
        else
        {
            {
                fprintf(stdout, "FAIL");
                return -1;
            }
        }
        break;

    default:
        print_usage(argv[0]);
        break;
    }
}
