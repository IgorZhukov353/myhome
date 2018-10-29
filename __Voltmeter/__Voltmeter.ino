int analogInput = 0;
float vout = 0.0;
float vin = 0.0;
float R1 = 96600.0; // resistance of R1 (100K) -see text!
float R2 = 11600.0; // resistance of R2 (10K) — see text!
int value = 0;
int pinVal = 1;

void setup(){
pinMode(analogInput, INPUT);
pinMode(13, OUTPUT);
Serial.begin(115200);
}

void loop(){
   digitalWrite(13, pinVal);
   pinVal = !pinVal;
// read the value at analog input
value = analogRead(analogInput);
vout = (value * 4.6) / 1024.0; // see text
vin = vout / (R2/(R1+R2));
if (vin<0.09) {
vin=0.0;//statement to quash undesired reading !
}
Serial.println("VIN=" + String(vin));
delay(500);
}
