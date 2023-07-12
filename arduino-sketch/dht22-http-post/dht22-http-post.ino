#include "DHT.h"
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include "secrets.h"

// Wifi settings ============================
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char serverAddress[] = "192.168.3.142";
WiFiClient wifi;
int status = WL_IDLE_STATUS;

// HTTP settings ============================
int port = 8000;
HttpClient client = HttpClient(wifi, serverAddress, port);
bool timeTracked = false;

// DHT settings ============================
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// System variables ========================
int serverInitialTime = 0;
int localInitialTime = 0;

void setup() {
  client.setTimeout(1);
  // WIFI Setup ============================
  Serial.begin(9600);
  delay(5000);
  while (status != WL_CONNECTED) {
    Serial.print("Connecting to network: ");
    Serial.println(ssid);
    delay(1000);
    status = WiFi.begin(ssid, pass);
    delay(1000);
    Serial.println(status);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  // DHT Setup ============================
  dht.begin();
}

void loop() {
  // POST Request
  String contentType = "application/x-www-form-urlencoded";
  if(!timeTracked){
    Serial.println("syncing timestamp");
    delay(1000);
    // POST Request
    String postData = "";
    client.post("/sync", contentType, postData);
    String response = client.responseBody();
    // Maybe also eval if response.length is large
    if(response != "-2"){
      serverInitialTime = response.toInt();
      localInitialTime = millis();
      timeTracked = true;
      Serial.print("Server Init time: ");
      Serial.print(serverInitialTime);
      Serial.println();
    }
  }
  else{
    // Wait a few seconds between measurements.
    delay(2000);
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.println();

    // POST Request
    String postData = "temperature=" + String(t) + "&humidity=" + String(h) + "&timestamp=" + String(serverInitialTime + millis() - localInitialTime);
    client.post("/test", contentType, postData);
    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
  }
}
