#include <ESP8266WiFi.h>              // Include main library
#include "DHT.h";
#include <Wire.h>
#include <BH1750.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET -1
#define OLED_ADDR   0x3C
Adafruit_SSD1306 OLED(OLED_RESET);

#define DHTTYPE DHT11
#define DHTPIN D4
DHT dht(DHTPIN, DHTTYPE, 15);

#define lamp D0
#define LEDFLASHER D5
#define ALARMSOUNDER D7 

BH1750 lightMeter;

const char* ssid     = "GZLW";    // Set your SSID
const char* password = "gggggggg";  // Set your password
const char* thing = "gzlw";   // Set Thing name
const char* host = "dweet.io";        // Set host 

int count = 0;
int sensorValue = 0;

float hum = 0;
float temp = 0;

long distance = 0;
long dis_val;
int statuss = 0;
#define tp D5
#define ep D6


void setup() 
{
  
  pinMode(lamp, OUTPUT); 
  pinMode(LEDFLASHER, OUTPUT); 
  pinMode(ALARMSOUNDER, OUTPUT);
  pinMode(tp, OUTPUT);
  pinMode(ep, INPUT);
  
  Serial.begin(115200);
  OLED.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  Wire.begin();
  lightMeter.begin();
  delay(10);
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");                // Print WiFi status
  }
  Serial.println("");
  Serial.println("WiFi connected");   // Print connect status
  Serial.println("IP address: ");     
  Serial.println(WiFi.localIP()); // Print IP address
}



void loop() 
{
  uint16_t lux = lightMeter.readLightLevel();
  sensorValue = lux;
  if (sensorValue < 3)
    {
        analogWrite(lamp, 255);
        statuss = 1;
    }
    else
    {
        analogWrite(lamp, 0);
        statuss = 0;
    }
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if(isnan(h) || isnan(t) || isnan(f)){
      Serial.println("Failed to read from DHT sensor!");
    }
  else{  
      hum = h;
      temp = t;
    }
  
  

  distance = read_hc_sr04();
  if(distance <= 200 && distance >= 0){
    dis_val = distance;
  }
  alarm(dis_val);
  
  
  ++count;                            // Count value
  
  WiFiClient client;
  const int httpPort = 80;            // Set HTTP port
  if (!client.connect(host, httpPort))// Test connection 
  {
    Serial.println("connection failed"); // Print connection fail message
    return;
  }
  String url =  String("/dweet/for/") + thing + "?"; // Set message
  url += "count=";
  url += count;
  url += "&value=";
  url += sensorValue;
  url += "&h=";
  url += hum;
  url += "&t=";
  url += temp;
  url += "&distance=";
  url += dis_val;
  url += "&statuss=";
  url += statuss;

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: keep-alive\r\n" +
               "Cache-Control: max-age=0\r\n\r\n");
  Serial.println("URL : ");         
  Serial.println(url);              // Print URL 
  delay(3000);

  
  while (client.available()) 
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection"); // Print closing status

  OLED.clearDisplay();
    OLED.setTextColor(WHITE);
    OLED.setCursor(0, 0);
    OLED.setTextSize(2);
    OLED.println(String("H: ") + String(h, 0) + " %");
    OLED.setCursor(0, 30);
    OLED.print(String("T: ") + String(t, 0) + " ");
    OLED.write(247);
    OLED.print("C");
    OLED.display();
}


long read_hc_sr04(){
  long dura, dis;
  digitalWrite(tp, LOW);
  delayMicroseconds(2);
  digitalWrite(tp, HIGH);
  delayMicroseconds(10);
  digitalWrite(tp, LOW);
  dura = pulseIn(ep, HIGH);
  dis = (dura/2)/29.1;
  return dis;
}

void alarm(long distance){
  int i = 0;
    if (distance < 5)
    {

        for(i = 1; i <= 10; i++)
        {
            analogWrite(LEDFLASHER, 255);
            analogWrite(ALARMSOUNDER, 200);
            delay(100);
            analogWrite(LEDFLASHER, 0);
            analogWrite(ALARMSOUNDER, 25);
            delay(100);
        }
    }
    else
    {
        analogWrite(ALARMSOUNDER, 0);
        analogWrite(LEDFLASHER, LOW);
    }
}



