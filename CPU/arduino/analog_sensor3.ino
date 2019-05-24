#define HEADER_SIZE 4 // AA55AA55
#define SIPM_BUF_SIZE 64 // 64 channels, two byte
#define OTHER_SENSORS 4 // 4 channels, two byte
#define TOTAL_BUF_SIZE SIPM_BUF_SIZE+OTHER_SENSORS
#define DELAY 100 // ms

unsigned int iteration_number=0;

unsigned int data_buffer[TOTAL_BUF_SIZE]; //16 bits in arduino

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  iteration_number=0;
}


unsigned int simple_crc()
{
  unsigned int temp_crc=0;
  unsigned int ijk;
  for (ijk=0;ijk<TOTAL_BUF_SIZE;ijk++)
  {
    temp_crc+=data_buffer[ijk];
  }
  
  return (temp_crc);
  
}

void loop() {
  iteration_number++;
  unsigned int ijk;
  unsigned int crc=0;
   // read the input on analog pin 0, 1 and 2:
  int SiPM_sensor = analogRead(A0);
  int VIS_sensor = analogRead(A1);
  int UV_sensor = analogRead(A2);
  int Geiger_sensor = analogRead(A3);
  data_buffer[0]=SiPM_sensor;
  data_buffer[1]=VIS_sensor;
  data_buffer[2]=UV_sensor;
  data_buffer[3]=Geiger_sensor;
  
  for (ijk=0;ijk<SIPM_BUF_SIZE;ijk++)
  {
    data_buffer[ijk+OTHER_SENSORS]=ijk;
  }
  crc=simple_crc();
  Serial.write(0xAA);
  Serial.write(0x55);
  Serial.write(0xAA);
  Serial.write(0x55);
  
    Serial.write(iteration_number>>8);    
    Serial.write(iteration_number&0x00FF);
    
//   Serial.write(data_buffer,TOTAL_BUF_SIZE)
   for (ijk=0;ijk<TOTAL_BUF_SIZE;ijk++)
   {
    Serial.write(data_buffer[ijk]>>8);
    Serial.write(data_buffer[ijk]&0x00FF);
   }
    
    Serial.write(crc>>8);    
    Serial.write(crc&0x00FF);

    
 delay(DELAY); /* 0,1 sec delay */
 }
