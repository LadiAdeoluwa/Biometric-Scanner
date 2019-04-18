#include "define.h"
#include "command.h"
#include "stdio.h"
#include "stdlib.h"
#include "wiringPi.h"     //load WiringPi library
#include "wiringSerial.h" //load WiringPi serial library
#include <string.h>

static void print_usage(const char *pcProgramName)
{
    printf("Usage: %s not run properly\nExamples : %s open\n          %s close\n           %senrol\n          %sisPressfinger\n", pcProgramName);
}

int var;

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
        exit(1); //for wiringPi GPIO

    //check UART coummunication about WiringPi library,Raspberry and Fingerprint
    if (wiringPiSetup() < 0)
    {
        printf("WiringPi-UART error!");
        return -1;
    }

    if ((var = serialOpen("/dev/ttyS0", 9600)) < 0)
    {
        printf("Raspberry-UART error!");
        return -1;
    }

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

    else if (strcmp(command, "enrol") == 0)
    {
        switchNum = 4;
    }

    else if (strcmp(command, "identify") == 0)
    {
        switchNum = 5;
    }

    else if (strcmp(command, "close") == 0)
    {
        switchNum = 6;
    }
    else if (strcmp(command, "enroll") == 0)
    {
        switchNum = 7;
    }

    switch (switchNum)
    {
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
    case 2:
    {
        LED_open();
        INT delay_seuense = argv[2] || 0;
        delay(delay_seuense);
        IsPressFinger();
        if (returnParameter != 0x1012)
        {

            fprintf(stdout, "SUCCESS FINGER");
            return -1;
        }
        else
        {
            fprintf(stdout, "FAILED FINGER");
            return -1;
        }
        break;
    }
    case 3:
    {

        int loop_time = 0;
        const char *startCommand = argv[2];

        if (strcmp(startCommand, "host") == 0)
        {
            loop_time = -1;
        }
        else
        {
            loop_time = 0;
        }

        EnrollStart(loop_time);
        if (returnAck != ACK && returnParameter == 0x1005) //change another IDs if default ID=0 is occupied
        {
            for (loop_time = 1; loop_time <= 199; loop_time++) //found another IDs
            {
                EnrollStart(loop_time);
                if (returnAck == ACK)
                    break;
            }
            fprintf(stdout, "ENROLL START ::%d", loop_time);
            return -1;
        }
        else if (returnAck == ACK)
        {
            fprintf(stdout, "ENROLL START ::%d", loop_time);
            return -1;
        }
        break;
    }
    case 4:
    {
        const char *input = argv[2];

        int instance = 0;
        if (strcmp(input, "1") == 0)
        {
            instance = 1;
        }
        else if (strcmp(input, "2") == 0)
        {
            instance = 2;
        }
        else if (strcmp(input, "3") == 0)
        {
            instance = 3;
        }

        int loop_time = 1;
        while (1)
        {

            // IsPressFinger();
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

        Enroll(instance);
        if (returnAck != ACK)
        {
            fprintf(stdout, "ENROLL FAILED ::%d", instance);
            LED_close();
        }
        else if ((returnAck == ACK && (returnParameter >= 0 || returnParameter < 199)))
        {
            // while (1)
            // {
            //     IsPressFinger();
            //     if (returnParameter == 0x1012) //finger is not pressed
            //         break;
            // }
            fprintf(stdout, "ENROLL SUCCESS ::%d", instance);
            LED_close();
        }
        delay(1500);
        LED_open();
        return -1;
    }

    case 5:

        LED_open();
        IsPressFinger();
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

        Identify();
        if (returnAck == ACK)
        {
            fprintf(stdout, "SUCCESS ::%ld", returnParameter);
            LED_close();
            delay(200);
            return 0;
        }
        else
        {
            fprintf(stdout, "FAILURE: IDENTIFY");
            LED_close();
            delay(200);
            ;
        }
        break;

    case 6:

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

    case 7:
    {
     

printf("hhhh");
        const char *input = argv[2];

        int instance = 0;
        if (strcmp(input, "1") == 0)
        {
            instance = 71;
        }
        else if (strcmp(input, "2") == 0)
        {
            instance = 72;
        }
        else if (strcmp(input, "3") == 0)
        {
            instance = 73;
        }
        printf("hhhh");
        int loop_time = 1;
        while (1)
        {

            // IsPressFinger();
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

        printf("outside loop");

        switch (instance)
        {
        case 71:
            Enroll1();
            if (returnAck != ACK)
            {
                fprintf(stdout, "ENROLL FAILED ::%d", instance);
                LED_close();
            }
            else if ((returnAck == ACK))
            {
                // while (1)
                // {
                //     IsPressFinger();
                //     if (returnParameter == 0x1012) //finger is not pressed
                //         break;
                // }
                fprintf(stdout, "ENROLL SUCCESS ::%d", instance);
                LED_close();
            }
            delay(1500);
            LED_open();
            return -1;

        case 72:
            Enroll2();
            if (returnAck != ACK)
            {
                fprintf(stdout, "ENROLL FAILED ::%d", instance);
                LED_close();
            }
            else if ((returnAck == ACK))
            {
                // while (1)
                // {
                //     IsPressFinger();
                //     if (returnParameter == 0x1012) //finger is not pressed
                //         break;
                // }
                fprintf(stdout, "ENROLL SUCCESS ::%d", instance);
                LED_close();
            }
            delay(1500);
            LED_open();
            return -1;

        case 73:
            Enroll3();
            if (returnAck != ACK)
            {
                fprintf(stdout, "ENROLL FAILED ::%d", instance);

            }
            else if ((returnAck == ACK))
            {
                // while (1)
                // {
                //     IsPressFinger();
                //     if (returnParameter == 0x1012) //finger is not pressed
                //         break;
                // }
                fprintf(stdout, "ENROLL SUCCESS ::%d", instance);
                LED_close();
            }
            delay(1500);
            LED_open();
            return -1;
        }
    }

    default:
        print_usage(argv[0]);
        break;
    }
}