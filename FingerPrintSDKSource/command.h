//TYPE DEFINITION
typedef unsigned int INT;
typedef unsigned char CHAR;
typedef unsigned long LONG;
typedef unsigned short SHORT;

//FUNCTION DEFINITION
void Open();
void Close();
void LED_open();
void LED_close();
void EnrollStart(int specify_ID);
void Enroll1();
void Enroll2();
void Enroll3();
void IsPressFinger();
void CaptureFinger(LONG picture_quality);
void GetTemplate(int specify_ID);
