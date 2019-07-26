#define sensor1 A0
#define sensor2 A1
#define sensor3 A3

void setup()
{
    Serial.begin(115200);
    pinMode(sensor1, INPUT);
    pinMode(sensor2, INPUT);
    pinMode(sensor3, INPUT);
}

void loop()
{
    int data1 = analogRead(A0);
    int data2 = analogRead(A1);
    int data3 = analogRead(A3);

    sendToPC(&data1, &data2, &data3);
    delay(100);
}

void sendToPC(int *data1, int *data2, int *data3)
{
    // uint8_t *byteData1 = (uint8_t *)(data1);
    // uint8_t *byteData2 = (uint8_t *)(data2);
    // uint8_t *byteData3 = (uint8_t *)(data3);
    // uint8_t buf[12] = {byteData1[0], byteData1[1], byteData1[2], byteData1[3],
    //                    byteData2[0], byteData2[1], byteData2[2], byteData2[3],
    //                    byteData3[0], byteData3[1], byteData3[2], byteData3[3]};
    // Serial.write(buf, 12);

    byte *byteData1 = (byte *)(data1);
    byte *byteData2 = (byte *)(data2);
    byte *byteData3 = (byte *)(data3);
    byte buf[6] = {byteData1[0], byteData1[1],
                   byteData2[0], byteData2[1],
                   byteData3[0], byteData3[1]};
    Serial.write(buf, 6);
}
