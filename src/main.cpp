// PINOUT ESP32 WeMoS LOLIN32 LITE
// https://forum.arduino.ru/uploads/default/original/1X/5e312ebaff8027632af71eff813c611c9699c0e5.jpeg

// SENSOR DE TEMPERATURA E UMIDADE DHT11 + FIREBASE

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>

#define DHTPIN 13
#define PINLED 2

// CONFIGURAÇÃO DHT:
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float tempAr;
float umidade;

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "escolaz"
#define WIFI_PASSWORD "147953Pietro@"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBztAQblHabEZmSEBnV1XPfYoL12KV259k"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "projectiot-f6a29-default-rtdb.firebaseio.com"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

int ledOnLine = 0;

void dhtSensor()
{
    // limpa as variáveis:
    tempAr = 0;  // Limpo variável de temperatura a cada nova leitura
    umidade = 0; // Limpo variável de umidade a cada nova leitura

    // Leitura do sensor
    tempAr = dht.readTemperature(); // Função que realiza a leitura da temperatura do sensor dht11
    umidade = dht.readHumidity();   // Função que realiza a leitura da umidade do sensor dht11

    // Caso os valores lidos do sensor não sejam válidos executamos as seguintes linhas
    if (isnan(tempAr) || isnan(umidade))
    {
        Serial.println("Falha na leitura do sensor DHT"); // Informar ao usuário que houve uma falha na leitura do sensor
        delay(1500);                                      // Aguardamos 1,5 segundos
        return;                                           // Retornamos a função para o seu ponto inicial
    }

    Serial.print("Temperatura Ambiente: ");
    Serial.println(tempAr);
    Serial.print("Umidade do Ar: ");
    Serial.println(umidade);
}

void piscaLed()
{
}

void setup()
{
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Sign up */

    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("ok");
        signupOK = true;
    }
    else
    {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void loop()
{
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();
        // Write an Int number on the database path test/int

        dhtSensor();

        if (Firebase.RTDB.setFloat(&fbdo, "ESP32/TEMP", tempAr))
        {
            Serial.print("PASSED");
            Serial.print(" Pasta:" + fbdo.dataPath());
            Serial.println(" tipo:" + fbdo.dataType());
        }
        else
        {
            Serial.println("Falha:" + fbdo.errorReason());
        }

        // Write an Float number on the database path test/float
        if (Firebase.RTDB.setFloat(&fbdo, "ESP32/UMI", umidade))
        {
            Serial.print("PASSED");
            Serial.print(" Pasta:" + fbdo.dataPath());
            Serial.println(" tipo:" + fbdo.dataType());
        }
        else
        {
            Serial.println("Falha:" + fbdo.errorReason());
        }
        Firebase.RTDB.setTimestamp(&fbdo, "/ESP32/DATE_UPDATE");
        Serial.println("tempo:" + fbdo.dataPath());

        // ledOnLine = Firebase.RTDB.getInt(&fbdo, "/ESP32/LedOnLine");

        Serial.println("Led:" + ledOnLine);

        digitalWrite(PINLED, ledOnLine);
    }
}