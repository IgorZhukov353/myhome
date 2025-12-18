
void setup() {
  // put your setup code here, to run once:

}
int i=0;
void loop() {
  // put your main code here, to run repeatedly:


  i=!i;
  digitalWrite(8,i);
  delay(1000);
}
