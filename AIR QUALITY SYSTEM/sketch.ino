#define BLYNK_TEMPLATE_ID "TMPL6A-lmRL5f"
#define BLYNK_TEMPLATE_NAME "airquality"
#define BLYNK_AUTH_TOKEN "TxpwOVSJZRZ2dA2tmvxOY1qi1AoGBAg4"

#define FAN       26
#define GLED      18
#define RLED      19
#define DUST      5
#define MQ7       33
#define DHTPIN    23
#define TESTP     35

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include <string.h> 
#include <DHT.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHT22);

String alert1 = "", alert2 = "", alert3 = "", alert4 = "";
float coAmount, dustAmount, Humidity, Temperature;
const int maxTemperature = 50, maxcoAmount = 50, maxdustAmount = 50;
String notification;

unsigned long duration;
unsigned long starttime = 0;
unsigned long endtime = 10 ;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
bool gasDetected = false; // Biến trạng thái phát hiện khí Gas hoặc CO

void setup()
{
  lcd.init();                  
  lcd.backlight();
  loadingdisplay();
  dht.begin();
  Serial.begin(9600);
 
  pinMode(GLED, OUTPUT);
  pinMode(RLED, OUTPUT);
  pinMode(FAN, OUTPUT);
  digitalWrite(FAN, LOW);
 
  Blynk.begin(auth, ssid, pass);
  digitalWrite(GLED, HIGH);
  digitalWrite(RLED, LOW);
}

void loop()
{
  Blynk.run();
  checkCO();
  checkDust();
  tempHum();

  // Kiểm tra khí Gas hoặc CO
  if (coAmount > maxcoAmount)
  {
    gasDetected = true;
    alert_display("CO GAS DETECTED");
    notification = "DANGEROUS GAS DETECTED! MOVE AWAY!";
    activateRedBlinking(); // Nhấp nháy LED đỏ
  }
  else if (dustAmount > maxdustAmount)
  {
    digitalWrite(GLED, LOW);
    digitalWrite(RLED, HIGH);
    digitalWrite(FAN, HIGH);
    alert_display("DUST DETECTED");
    notification = "DANGEROUS DUST DETECTED! MOVE AWAY!";
    delay(3000); // Giữ trạng thái trong 3 giây
  }
  else if ((Temperature >= 12 && Temperature <= 25) || Temperature > 36 ||
           (Humidity >= 10 && Humidity <= 20) || Humidity > 50)
  {
    // Bật LED đỏ và quạt
    digitalWrite(GLED, LOW);
    digitalWrite(RLED, HIGH);
    digitalWrite(FAN, HIGH);
  }
  else
  {
    // Tắt trạng thái cảnh báo nếu bình thường
    gasDetected = false;
    digitalWrite(RLED, LOW);
    digitalWrite(GLED, HIGH);
    digitalWrite(FAN, LOW);
  }

  homedisplay();  // Hiển thị thông tin lên LCD
  send_update();  // Gửi dữ liệu đến Blynk
}

void activateRedBlinking()
{
  for (int i = 0; i < 5; i++) // Nhấp nháy 5 lần
  {
    digitalWrite(RLED, HIGH);
    delay(500);
    digitalWrite(RLED, LOW);
    delay(500);
  }
  digitalWrite(RLED, HIGH); // Giữ LED đỏ sau khi nhấp nháy
}

void send_update() 
{     
  Blynk.virtualWrite(V0, Temperature);
  Blynk.virtualWrite(V1, Humidity);  
  Blynk.virtualWrite(V2, dustAmount);
  Blynk.virtualWrite(V3, coAmount);
  Blynk.virtualWrite(V4, notification);
  delay(150);  
}

void loadingdisplay()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AIRQUALITY--> "); 
  lcd.setCursor(0, 1);
  lcd.print("LOADING..."); 
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WAIT A SEC");
  lcd.setCursor(0, 1);
  for (int k = 0; k < 16; k++)
  {
    lcd.setCursor(k, 1);
    lcd.print(".");
    delay(500);
  }
}

void homedisplay()
{ 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:"); 
  lcd.print(Temperature, 1);
  lcd.setCursor(7, 0);
  lcd.print("H:"); 
  lcd.print(Humidity, 1);
  lcd.setCursor(0, 1);
  lcd.print("D:"); 
  lcd.print(dustAmount, 1);
  lcd.setCursor(7, 1);
  lcd.print("G:"); 
  lcd.print(coAmount, 1);
  delay(500);
}

void alert_display(String textAlert)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("EMERGENCY ALERT!!!");
  lcd.setCursor(0, 1);
  lcd.print(textAlert);
}

void checkDust()
{
  duration = pulseIn(DUST, LOW);
  lowpulseoccupancy += duration;
  endtime = millis();
  if ((endtime - starttime) > sampletime_ms)
  {
    ratio = (lowpulseoccupancy * 1.0) / sampletime_ms;
    concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62;
    dustAmount = map(analogRead(TESTP), 0, 4095, 0, 100);
    Serial.print("lowpulseoccupancy: ");
    Serial.print(lowpulseoccupancy);
    Serial.print(" ratio: ");
    Serial.print(ratio);
    Serial.print(" GP2Y1010AU0F: ");
    Serial.println(concentration);
    lowpulseoccupancy = 0;
    starttime = millis();
  }
}

void checkCO()
{
  int sensorValue = analogRead(MQ7);
  Serial.print(sensorValue / 60, DEC);
  coAmount = map(sensorValue, 0, 4095, 0, 100);
}

void tempHum()
{

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" Humidity: "); 
  Serial.print(h);
  Temperature = t;
  Humidity = h;
}
