#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <RTClib.h>

#define ph_pin 4

RTC_DS3231 rtc;

String ph_sensor();
String rtclock();
void particle();
void wifi_connect(String ssid_func, String pass_func);

AsyncWebServer server(80);
String ssid;
String password;
int selection = 0;


void setup() {
  Serial.begin(115200);

  pinMode(ph_pin, INPUT);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  rtc.adjust(DateTime(__DATE__, __TIME__));
}

void loop() {
  switch (selection)
  {
  case 0:
    if(Serial.read() > 1){
      ssid = Serial.readString();
      Serial.flush();
      selection = 1;
    }
  case 1:
    if(Serial.read() > 1){
      password = Serial.readString();
      Serial.flush();
      selection = 2;
    }
  case 2:
      wifi_connect(ssid, password);
      selection = 3;
    break;
  
  default:
    break;
  }
}

String rtclock(){
  DateTime now = rtc.now();
  int year = now.year();
  int month = now.month();
  int hora = now.hour();
  return String(hora + ":" +  month + year);
}

String ph_sensor(){
  unsigned long int avg;
  float reads[10], temp;
  float ph_value;
  for(int i = 0; i < 10; i++){
    reads[i] = analogRead(ph_pin);
    delay(50);
  }
  for(int i = 0; i < 9; i++){
    for(int j = i + 1; j < 10; j++){
      if(reads[i] > reads[j]){
        temp = reads[i];
        reads[i] = reads[j];
        reads[j] = temp;
      }
    }
  }

  avg = 0;
  for(int i = 2; i < 8; i++)
    avg += reads[i];
  ph_value = (float)avg*5.0/1024/6;
  ph_value = 3.5*ph_value;
  delay(1000);

  return String(ph_value);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 demo Server</h2>
  <p>
    <i class="Ph sensor" style="color:#059e8a;"></i> 
    <span class="labels">PH</span> 
    <span id="PH">%PH%</span>
  </p>
  <p>
    <i class="RTC clock esp" style="color:#00add6;"></i> 
    <span class= "labels">Clock</span>
    <span id="CLOCK">%CLOCK%</span>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("PH").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/PH", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("CLOCK").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/clock", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "PH"){
    return ph_sensor();
  }
  else if(var == "CLOCK"){
    return rtclock();
  }
  return String();
}

void wifi_connect(String ssid_func, String pass_func){
  WiFi.begin(ssid_func.c_str(), pass_func.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
}