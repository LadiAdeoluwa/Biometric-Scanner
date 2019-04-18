typedef unsigned int   INT;
typedef unsigned char  CHAR;
typedef unsigned long  LONG;
typedef unsigned short SHORT;

void Open();
void Close ();
void LED_open();
void LED_close();
void GetEnrollCount();
void CheckEnrolled (int specify_ID);
void ChangeBaudRate(int specify_ID);
void EnrollStart(int specify_ID);
void Enroll(int Enroll_define);
void Enroll1();
void Enroll2();
void Enroll3();
void EnrollStart1(int specify_ID);
void MakeTemplate();
void IsPressFinger();
void DeleteId (int specify_ID);
void DeleteAll();
void Identify();
void CaptureFinger(LONG picture_quality);
void GetImage ();
void GetTemplate(int specify_ID);
void SetTemplate(int specify_ID);
void GetDatabase();
void SetDatabase();
void DeviceSerialNumber();
void VerifyTemplate(int specify_ID);
void IdentifyTemplate();
void VerifyTemplate1(); 
void VerifyTemplate2(char filename[64],int specify_ID);


