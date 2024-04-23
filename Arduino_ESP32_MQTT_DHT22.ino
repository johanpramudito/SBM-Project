#include <WiFi.h>  
#include <PubSubClient.h>
#include <DHT.h>

const int DHT_PIN = 5;  
const char* ssid = "masukkan nama wifi"; ///  wifi ssid 
const char* password = "masukkan password wifi";
const char* mqtt_server = "masukkan IP laptop yang tersambung dengan wifi";// mosquitto server url

DHT sensor_dht(DHT_PIN, DHT22);
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;

void setup_wifi() { 
  delay(10);
  Serial.println();
  Serial.print("Wifi terkoneksi ke : ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi berhasil terkoneksi");
  Serial.print("Alamat IP : ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) { 
    Serial.print((char)payload[i]);
  }
}

void reconnect() { 
  while (!client.connected()) {
    Serial.print("Baru melakukan koneksi MQTT ...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  sensor_dht.begin();
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 
}   

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); //connect mqtt

  unsigned long now = millis();
  if (now - lastMsg > 2000) { //perintah publish data
    lastMsg = now;
    
    float temp = sensor_dht.readTemperature();
    float hum = sensor_dht.readHumidity();

    char temp_str[8];
    dtostrf(temp, 1, 2, temp_str);
    client.publish("jhnizl/temperature", temp_str); // publish temp topic terpisah
    char hum_str[8];
    dtostrf(hum, 1, 2, hum_str);
    client.publish("jhnizl/humidity", hum_str);   // publish hum topic terpisah

    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(hum);
    
    char jsonString[100];
    snprintf(jsonString, sizeof(jsonString), "{\"temperature\": %.2f, \"humidity\": %.2f}", temp, hum);
    
    client.publish("jhnizl/sensordata", jsonString); // publish sensordata gabung

    Serial.println("Data published:");
    Serial.println(jsonString);
    
    delay(2000); // Delay sebelum publish berikutnya
  }
}
