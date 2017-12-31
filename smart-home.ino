#include <Servo.h>
// constants
#define WiFi Serial2
const String SSID = "";
const String Password = "";
const String Host = "your-host-name";
const int Port = 80;
const String api = "your-api";
const int motionSensorInPin = 12;
const int motionSensorOutPin = 13;
const int gasSensorInPin = A0;
const int gasSensorOutPin = 2;
const int temperatureInPin = A2;
const int temperatureOutPin = 11;
const int servoPin = 7;
const int led = 8;

// your-url to get pins' values for example
String URL = "/getPinValue.php?api=" + api + "&pinNum=*";
String data;
char bluetoothChar = 'S';
boolean motionSensorReading;
int gasSensorReading;
int temperatureSensorReading;
float temperatureInC;
Servo doorServo;
int servoPosition = 180;

void setup() {
  for(short i = 22; i < 54; i++)
    pinMode(i, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(115200);
  // Wi-Fi
  wifiBegin();
  // Motion Sensor
  pinMode(motionSensorInPin, INPUT);
  pinMode(motionSensorOutPin, OUTPUT);
  // Gas Sensor
  pinMode(gasSensorOutPin, OUTPUT);
  pinMode(gasSensorInPin, INPUT);
  // Temperature
  pinMode(temperatureOutPin, OUTPUT);
  // Servo
  doorServo.attach(5);
  // led
  pinMode(led, OUTPUT);
}

void loop() {
  // Wi-Fi Mode
  unsigned long start = millis();
  Serial.println("---------- START Sending Request -----------");
  data = GetDataFrom(URL);
  for(short i = 0; i < data.length(); i++){
    if(data[i] == '#') break;
    int pin = i + 22;
    int volt = int(data[i] - 48);
    digitalWrite(pin, volt);
    Serial.print(pin);
    Serial.print(" - ");
    Serial.println(volt);
  }
  Serial.println("----------- END Request Response -----------");
  Serial.println(millis() - start);
  // Motion Sensor
  motionSensorReading = digitalRead(motionSensorInPin);
  if (motionSensorReading == true) {
    digitalWrite(motionSensorOutPin, HIGH);
    Serial.println(">> There is motion");
  } else if (motionSensorReading == false) {
    digitalWrite(motionSensorOutPin, LOW);
    Serial.println(">> No motion");
  }
  // Gas Sensor
  gasSensorReading = analogRead(gasSensorInPin);
  Serial.println(gasSensorReading);
  if(gasSensorReading > 500) {
    digitalWrite(gasSensorOutPin, HIGH);
    Serial.println(">> There is fire");
  } else if(gasSensorReading < 400) {
    digitalWrite(gasSensorOutPin, LOW);
    Serial.println(">> No fire");
  }
  // Temperature Sensor
  temperatureSensorReading = analogRead(temperatureInPin);
  temperatureInC = temperatureSensorReading / 9.31;
  Serial.println(temperatureInC);
  if(temperatureInC > 65) {
    digitalWrite(temperatureOutPin, HIGH);
    Serial.println(">> Temperature is HIGH");
  } else if (temperatureInC < 60) {
    digitalWrite(temperatureOutPin, LOW);
    Serial.println(">> Temperature is LOW");
  }
}

void wifiBegin(){
  while(!WiFiIsReady()) Serial.println("?? Module Is NOT Ready.");
  Serial.println(">> Module Is Ready.");

  while(!WiFiModeIsSet()) Serial.println("?? Module Mode Is NOT Set.");
  Serial.println(">> Module Mode Is Set.");

  while(!WiFiConnectToNetwork()) Serial.println("?? Module Can NOT Connect to " + SSID + ".");
  Serial.println(">> Module Is Connected to " + SSID);

  while(!WiFiSetupTCP()) Serial.println("?? Module Can NOT Set TCP Mode. ");
  Serial.println(">> Module TCP Mode Is set");
}

bool WiFiIsReady(){
  WiFi.println("AT");
  return WiFi.find("OK");
}

bool WiFiModeIsSet(){
  WiFi.println("AT+CWMODE=3");
  return WiFi.find("OK");
}

bool WiFiConnectToNetwork(){
  WiFi.println("AT+CWJAP=\"" + SSID + "\",\"" + Password + "\"");
  return WiFi.find("OK");
}

bool WiFiSetupTCP(){
  WiFi.println("AT+CIPMUX=0");
  return WiFi.find("OK");
}

bool WiFiConnectedToHost(){
  WiFi.println("AT+CIPSTART=\"TCP\",\"" + Host + "\"," + Port);
  return WiFi.find("OK");
}

String GetDataFrom(String URL){
  while(!WiFiConnectedToHost()) Serial.println("?? Module Can NOT Connect to Host " + Host + ".");
  Serial.println(">> Module Is Connected To " + Host + ".");
  String receivedData = "";
  int maxRequests = 10;
  int Requests = 0;
  String cmd = "GET " + URL + " HTTP/1.0\r\nHost: " + Host + ":" + Port + "\r\n\r\n";
  WiFi.print("AT+CIPSEND=");
  WiFi.println(cmd.length());
  while(!WiFi.find('>')){
    Serial.println("-- Waiting 1st Response to Request ...");
    WiFi.print("AT+CIPSEND=");
    WiFi.println(cmd.length());
    Requests++;
    if(Requests == maxRequests) return "";
  }
  WiFi.println(cmd);
  Serial.println(cmd);
  while(!WiFi.find("OK")){
    Serial.println("-> Waiting 2nd Response to Request ...");
    Requests++;
    if(Requests == maxRequests) return "";
  }
  Serial.println(">> Request SENT");
  while(!WiFi.find("+IPD,")){
    Serial.println("-- Waiting Response ...");
    Requests++;
    if(Requests == maxRequests) return "";
  }
  Serial.println("-> Response detected");
  String allData = WiFi.readString();
  Serial.println(">> All Data");
  Serial.println(allData);
  for(unsigned int i = 0; i < allData.length(); i++)
    if(allData[i] == '\r')
      if(allData[i+1] == '\n')
        if(allData[i+2] == '\r')
          for(i+=4; i < allData.length() - 8; i++)
            receivedData += allData[i];
  Serial.println(">> Data has been Received");
  Serial.println(receivedData);
  return receivedData;
}
