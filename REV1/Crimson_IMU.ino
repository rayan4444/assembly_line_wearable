#define sensor A0

void setup()
{
    Serial.begin(115200);
    pinMode(sensor, INPUT);
}

void loop()
{
    int data = analogRead(A0);
    // Serial.println(data);
    sendToPC(&data);
    delay(100);
}

void sendToPC(int *val)
{
    uint8_t *p = (uint8_t *)(val); //casting to a byte pointer
    Serial.write(p, 2);
}
