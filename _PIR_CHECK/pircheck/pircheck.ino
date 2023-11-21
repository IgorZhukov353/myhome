const int movPin = 10;

void setup() {
    Serial.begin(115200);
    pinMode(movPin, INPUT);
}

void loop(){
    int val = digitalRead(movPin);
    Serial.println(val);
    delay(100);
}
