#define WP 7
#define CH 7
#define RW 0
#define CH_REG 0x80
#define WP_REG 0x8E


uint8_t sendByte();
uint8_t bcd_to_dec(uint8_t bcd);
uint8_t dec_to_bcd(uint8_t dec);
void set_time_date(uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t day, uint8_t month, uint8_t year);
void disp_time_date();

void setup() {
  // put your setup code here, to run once:
  pinMode(SS,OUTPUT);//SS is an output CE
  pinMode(MISO,INPUT);  //use only MISO pin for i/o data
  pinMode(SCK,OUTPUT);

  Serial.begin(115200);
  Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCK: ");
  Serial.println(SCK);
  Serial.print("SS: ");
  Serial.println(SS);
 
  //Serial.println(sendByte(0x8E, 0));//WP bit
  //Serial.println(sendByte(0x80, 0));//seconds and CH bit

  sendByte(WP_REG, 0);  //disable write protect
  sendByte(CH_REG, sendByte(CH_REG | 1<<RW, 0) & !(1<<CH));

  //set_time_date(0,42,17,26,7,11,23);


}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println(sendByte(0x85, 0), HEX);
  //Serial.println(sendByte(0x83, 0), HEX);
  //Serial.println(bcd_to_dec(sendByte(0x81, 0)));
  disp_time_date();


  if(Serial.available() == 5)
  {
    char readBuffer[5];
    char* pEnd;
    char out[200];
    long int cmd=0;
    long int data=0;

  //pierwsze dwa znaki jako jedna liczba, a kolejne jako druga

    Serial.readBytes(readBuffer, 5);
    cmd = strtol(readBuffer, &pEnd, 16);
    data = strtol(pEnd, &pEnd, 16);

    sprintf(out, "polecenie: %X, dana: %X, odebrane: %X", cmd, data, sendByte(cmd, data));
  
    Serial.println(out);
    //Serial.println(sCmd);
    //Serial.println(sData);
  }
  
  uint8_t time_date_size = 19;
  if(Serial.available() == time_date_size)
  {
    char readBuffer[time_date_size];
    uint8_t time_date[7];
    char* p;
    char out[200];

    Serial.readBytes(readBuffer, time_date_size);
    
    time_date[0] = strtol(readBuffer, &p, 10);
    time_date[1] = strtol(p, &p, 10);
    time_date[2] = strtol(p, &p, 10);
    time_date[3] = strtol(p, &p, 10);
    time_date[4] = strtol(p, &p, 10);
    time_date[5] = strtol(p, &p, 10);
    time_date[6] = strtol(p, &p, 10);



    set_time_date(time_date[2], time_date[1], time_date[0], time_date[3], time_date[4], time_date[5], time_date[6]);
  }


  delay(5000);
}


uint8_t sendByte(uint8_t cmd, uint8_t data)
{
  /*begin of transmission
  sending msb first
  */
  uint8_t rec_data=0; //received data
  bool read_data = false;
  int t = 1000;
  bool opz = false;

  if(cmd & 0x01) //if read data
  {
    read_data = true;
  }

  pinMode(MISO, OUTPUT);//entering cmd, setting as output
  digitalWrite(SCK, LOW);
  delayMicroseconds(10);
  if(opz)delay(t);

  digitalWrite(SS, HIGH);
  
  for(int i=0; i<8; i++)
  {
    if(cmd & 0x01)
    {
      digitalWrite(MISO, HIGH);
      //Serial.println("1");
    }
    else
    {
      digitalWrite(MISO, LOW);
       //Serial.println("0");
    }
    cmd >>= 1;
    delayMicroseconds(1);

    digitalWrite(SCK, HIGH);
    delayMicroseconds(2);
    if(opz)delay(t);
    digitalWrite(SCK, LOW);
    //delayMicroseconds(1);
    if(opz)delay(t);
  }
   //Serial.println("\n");

  //sending/receiving data
  if(read_data)pinMode(MISO, INPUT); //if read data change pin to input
  delayMicroseconds(1);

  for(int i=0; i<8; i++)
  {
    if(i<7)
    {
      if(digitalRead(MISO) == HIGH)rec_data |= 0x80;  //receive one bit from the rtc
      rec_data >>= 1;
    }

    if(!read_data) //if write data
    {
      if(data & 0x01)digitalWrite(MISO, HIGH);
      else digitalWrite(MISO, LOW);

      data >>= 1;
      delayMicroseconds(1);
    }
    
    digitalWrite(SCK, HIGH);
    delayMicroseconds(2);
    if(opz)delay(t);
    
    digitalWrite(SCK, LOW);
    //delayMicroseconds(1);
    if(opz)delay(t);

    delayMicroseconds(2);
  }
  digitalWrite(SS, LOW);

  return rec_data;
}

uint8_t bcd_to_dec(uint8_t bcd)
{
  return ((bcd & 0xF0)>>4)*10 + (bcd & 0x0F);
}

uint8_t dec_to_bcd(uint8_t dec)
{
  uint8_t digit1 = dec / 10;
  uint8_t digit2 = dec % 10;

  return (digit1 << 4) + digit2;
}

void set_time_date(uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t day, uint8_t month, uint8_t year)
{
  sendByte(0x82, dec_to_bcd(min));
  sendByte(0x84, dec_to_bcd(hour));
  sendByte(0x86, dec_to_bcd(date));
  sendByte(0x88, dec_to_bcd(month));
  sendByte(0x8A, dec_to_bcd(day));
  sendByte(0x8C, dec_to_bcd(year));
  sendByte(0x80, dec_to_bcd(sec));
}

void disp_time_date()
{
  uint8_t sec = bcd_to_dec(sendByte(0x81, dec_to_bcd(sec)));
  uint8_t min = bcd_to_dec(sendByte(0x83, dec_to_bcd(sec)));
  uint8_t hour = bcd_to_dec(sendByte(0x85, dec_to_bcd(sec)));
  uint8_t date = bcd_to_dec(sendByte(0x87, dec_to_bcd(sec)));
  uint8_t month = bcd_to_dec(sendByte(0x89, dec_to_bcd(sec)));
  uint8_t day = bcd_to_dec(sendByte(0x8B, dec_to_bcd(sec)));
  uint8_t year = bcd_to_dec(sendByte(0x8D, dec_to_bcd(sec)));

  Serial.print(hour);
  Serial.print(" : ");
  Serial.print(min);
  Serial.print(" : ");
  Serial.print(sec);
  Serial.print(", ");
  Serial.print(date);
  Serial.print(".  ");
  Serial.print(month);
  Serial.print(".  ");
  Serial.print(year);
  Serial.print(", day of week: ");
  Serial.println(day);
}
