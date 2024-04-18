#include <Arduino.h>
#include <WebServer.h>
#include "WiFiManager.h"
#include <ESP.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

WebServer server(80);

#define TFT_CS     22  // Chip select
#define TFT_RST    12  // Reset
#define TFT_DC     21  // Data/Command
#define TFT_MOSI   19  // DIN
#define TFT_SCLK   18  // CLK

String payloadStr;

// Configuración de botón y escaneo BLE
const int buttonPin = 35;              // Pin del botón
unsigned long buttonPressTime = 0;     // Tiempo cuando se presiona el botón
bool isPressing = false;               // Estado del botón (presionado/no presionado)
int scanTime = 30;                     // Tiempo de escaneo en segundos
BLEScan* pBLEScan;                     // Puntero a la instancia de BLEScan
// Dispositivos Bluetooth LE a buscar
String macAddresses[] = {"d7:e1:23:32:c4:5a", "e4:41:ad:7c:63:54"};
String humidityMacAddresses[] = {""};
int numberOfDevices = sizeof(macAddresses) / sizeof(macAddresses[0]);
int numberOfHumidityDevices = sizeof(humidityMacAddresses) / sizeof(humidityMacAddresses[0]);

// Gestión de la configuración WiFi
bool stopScan = false;
WiFiManager wiFiManager;               // Instancia de WiFiManager

String temperature = ""; // Variable para la temperatura
String humidity = ""; // Variable para la humedad

bool pantallaPrincipal = false;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

static const unsigned char PROGMEM wifi[] =
{
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000111, 0b11100000,
  0b00011111, 0b11111000,
  0b01110000, 0b00001110,
  0b11100011, 0b11000111,
  0b11001110, 0b01110011,
  0b00011000, 0b00011000,
  0b00010011, 0b11001000,
  0b00000111, 0b11100000,
  0b00000000, 0b00000000,
  0b00000001, 0b10000000,
  0b00000001, 0b10000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
};

static const unsigned char PROGMEM wifi_falla[] =
{
  0b11000000, 0b00000000,
  0b01100000, 0b00000000,
  0b00110000, 0b01100000,
  0b00011100, 0b00111000,
  0b01111110, 0b00011110,
  0b11111111, 0b00001111,
  0b11111111, 0b10000111,
  0b00011111, 0b11000000,
  0b00011111, 0b11100000,
  0b00000111, 0b11111000,
  0b00000011, 0b11101100,
  0b00000001, 0b10000110,
  0b00000001, 0b10000011,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
};

static const unsigned char PROGMEM grados[] =
{
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000111, 0b11000000,
  0b00001111, 0b11100000,
  0b00011000, 0b00110000,
  0b00011000, 0b00110000,
  0b00011000, 0b00110000,
  0b00011000, 0b00110000,
  0b00001111, 0b11100000,
  0b00000111, 0b11000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
};

static const unsigned char PROGMEM envia[] =
{
  0b00000000, 0b00000000,
  0b00000001, 0b10000000,
  0b00000011, 0b11000000,
  0b00000111, 0b11100000,
  0b00001111, 0b11110000,
  0b00011111, 0b11111000,
  0b00111111, 0b11111100,
  0b01111111, 0b11111110,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b00000000, 0b00000000,
};

static const unsigned char PROGMEM recibe[] =
{
  0b00000000, 0b00000000,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b00000111, 0b11100000,
  0b01111111, 0b11111110,
  0b00111111, 0b11111100,
  0b00011111, 0b11111000,
  0b00001111, 0b11110000,
  0b00000111, 0b11100000,
  0b00000011, 0b11000000,
  0b00000001, 0b10000000,
  0b00000000, 0b00000000,
};

static const unsigned char PROGMEM BT_icono[] = //Icono Bluetooth
{
  0b00000000, 0b10000000,
  0b00000000, 0b11000000,
  0b00000000, 0b11100000,
  0b00000000, 0b10110000,
  0b00001000, 0b10010000,
  0b00000110, 0b10110000,
  0b00000011, 0b11100000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b00000110, 0b10110000,
  0b00001000, 0b10010000,
  0b00000000, 0b10110000,
  0b00000000, 0b11100000,
  0b00000000, 0b11000000,
  0b00000000, 0b10000000,
};

static const unsigned char PROGMEM cuadro[] =
{
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00011111, 0b11111000,
  0b00011111, 0b11111000,
  0b00011111, 0b11111000,
  0b00011111, 0b11111000,
  0b00011111, 0b11111000,
  0b00011111, 0b11111000,
  0b00011111, 0b11111000,
  0b00011111, 0b11111000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
};

static const unsigned char PROGMEM logo_control_120px [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0xE0, 0x20, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0xE2, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x01, 0x21, 0x8C,
  0x00, 0x00, 0x00, 0x00, 0x21, 0xE0, 0x00, 0x1C, 0x80, 0x00, 0x08, 0x00, 0x01, 0x31, 0x88, 0x00,
  0x00, 0x00, 0x01, 0xE3, 0xE0, 0x04, 0x18, 0x00, 0x00, 0x0C, 0x00, 0x01, 0x33, 0x98, 0x00, 0x00,
  0x00, 0x01, 0xE3, 0xE0, 0x0E, 0x10, 0x0F, 0x13, 0x9C, 0x11, 0xE1, 0x13, 0x99, 0xF8, 0xB2, 0x40,
  0x01, 0xE3, 0xFF, 0xFE, 0x30, 0x11, 0x98, 0x88, 0xE3, 0x31, 0x12, 0x91, 0x18, 0xC6, 0x20, 0x01,
  0xE3, 0xE0, 0x0E, 0x10, 0x11, 0x98, 0x88, 0xC2, 0x11, 0x1E, 0x71, 0x08, 0x87, 0xE0, 0x00, 0x23,
  0xE0, 0x00, 0x18, 0x11, 0x98, 0x88, 0xC2, 0x11, 0x0E, 0x71, 0x08, 0x84, 0x00, 0x00, 0x31, 0xE0,
  0x00, 0x1F, 0x99, 0x18, 0x88, 0xC3, 0x31, 0x0C, 0x61, 0x98, 0x86, 0x00, 0x00, 0x10, 0xFF, 0x00,
  0x07, 0x8E, 0x18, 0x88, 0xC1, 0xE1, 0x0C, 0x61, 0xC8, 0x83, 0xC0, 0x00, 0x18, 0x01, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0xF0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x70, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0xE0, 0x70, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00,
};

static const unsigned char PROGMEM logo_avimex [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xF0, 0x03, 0x00, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x1F, 0xFF, 0xF8, 0x07, 0x80, 0x18, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0xFC, 0x07,
  0x80, 0x03, 0x33, 0x83, 0x86, 0x10, 0x3F, 0x00, 0x7C, 0x0F, 0xB8, 0xFB, 0xFF, 0xCF, 0xCF, 0x20,
  0x60, 0x0F, 0xFC, 0x1B, 0xB9, 0xB7, 0x39, 0xDC, 0xE7, 0x40, 0x41, 0xFF, 0xFC, 0x13, 0xB9, 0x77,
  0x39, 0xF8, 0xE7, 0x80, 0x0F, 0x01, 0xFC, 0x33, 0xBA, 0x76, 0x39, 0xFF, 0xE7, 0x80, 0x38, 0x1F,
  0xF8, 0x7F, 0xBE, 0x7E, 0x71, 0xF0, 0x07, 0x80, 0x61, 0xF7, 0xF0, 0x43, 0xBC, 0x6E, 0x73, 0xF0,
  0x0F, 0x80, 0xCF, 0x07, 0xE0, 0x83, 0xB8, 0xEE, 0x73, 0xF0, 0x1B, 0x80, 0x18, 0x3F, 0x81, 0x83,
  0xB8, 0xFC, 0x63, 0xB9, 0xB1, 0xC0, 0x31, 0xFE, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x20, 0x00,
  0x07, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};

void pantalla_principal() {
  tft.fillScreen(ST77XX_BLACK); // Limpia la pantalla con color negro
  tft.drawBitmap(5, 5, logo_avimex, 80, 23, ST77XX_WHITE);  // logo avimex
  tft.drawFastHLine(0, 45, 160, ST77XX_BLUE); // x,y
  tft.drawFastHLine(0, 46, 160, ST77XX_BLUE); // x,y

  tft.drawBitmap(140, 25, wifi_falla, 16, 16, ST77XX_BLACK);
  tft.drawBitmap(140, 5, wifi, 16, 16, ST77XX_GREEN);   // despliega wifi ok
}

void pantalla_falla(String text){
        tft.fillScreen(ST77XX_BLACK);
      tft.drawBitmap(140, 25, wifi_falla, 16, 16, ST77XX_CYAN);   // despliega wifi falla
      tft.drawBitmap(15, 20, logo_control_120px, 120, 31, ST77XX_WHITE);

      tft.setCursor(25, 64);   // x, y
      tft.setTextColor(ST77XX_RED);
      tft.setTextSize(1);
      tft.println(text);

      tft.setCursor(25, 84);   // x, y
      tft.setTextColor(ST77XX_RED);
      tft.setTextSize(1);
      tft.println("Wifi");
}

void startWiFiManager() {
  // Inicia WiFiManager para una nueva configuración
  wiFiManager.startConfigPortal("ControlWareTempHumiPro");
}

void ButtonWifiTask(void * parameter) {
  for (;;) {
    int buttonState = digitalRead(buttonPin);
    if (buttonState == LOW) {
      if (!isPressing) {
        isPressing = true;
        buttonPressTime = millis();
      } else if (millis() - buttonPressTime > 7000) {
        Serial.println("Botón presionado por 7 segundos. Borrando credenciales WiFi y reiniciando Equipo...");
        isPressing = false;
        // Desconectar y borrar credenciales WiFi anteriores
        WiFi.disconnect(true);
        delay(1000); // Pequeña pausa para asegurar que se complete la desconexión
        wiFiManager.erase();
        delay(1000); // Pequeña pausa para asegurar que se complete la desconexión
        ESP.restart();
      }
    } else {
      isPressing = false;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void parpadearIconoWiFi(int veces) {
  for (int i = 0; i < veces; i++) {
    // Dibuja el icono de WiFi
    tft.drawBitmap(140, 5, wifi, 16, 16, ST77XX_GREEN); // Dibuja el icono de WiFi
    delay(500); // Espera medio segundo

    // Borra el icono de WiFi
    tft.fillRect(140, 5, 16, 16, ST77XX_BLACK); // Borra el icono de WiFi
    delay(500); // Espera medio segundo
  }
  // Dibuja el icono de WiFi una vez más para dejarlo encendido
  tft.drawBitmap(140, 5, wifi, 16, 16, ST77XX_GREEN);
}
void parpadearFallaIconoWiFi(int veces) {
  for (int i = 0; i < veces; i++) {
    // Dibuja el icono de WiFi
    tft.drawBitmap(140, 5, wifi, 16, 16, ST77XX_RED); // Dibuja el icono de WiFi
    delay(500); // Espera medio segundo

    // Borra el icono de WiFi
    tft.fillRect(140, 5, 16, 16, ST77XX_BLACK); // Borra el icono de WiFi
    delay(500); // Espera medio segundo
  }
  // Dibuja el icono de WiFi una vez más para dejarlo encendido
  tft.drawBitmap(140, 5, wifi, 16, 16, ST77XX_RED);
}

void enviarDatos(String temp, String hum) {
  // Asegúrate de que estás conectado a WiFi antes de intentar enviar datos
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String numero_dispositivo = "572"; // Ajusta según sea necesario
    String mystring_status_pt100 = "OK"; // Ajusta según sea necesario
    String indicador_dispositivo = "2"; // Ajusta según sea necesario

    String url = "http://95.216.228.161/mccr_sensores/recibe_avimex.ashx?t=" + temp + "&d=" + numero_dispositivo + "&e=" + mystring_status_pt100 + "&c=0" + "&indicador=" + indicador_dispositivo;
    Serial.println(url);
    http.begin(url); // Inicia la conexión
    int httpCode = http.GET(); // Realiza la solicitud GET

    if(httpCode > 0) {
      // Si la solicitud fue exitosa, imprime el código de respuesta
      Serial.print("Código de respuesta: ");
      Serial.println(httpCode);
      if(httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        payloadStr = payload;
        parpadearIconoWiFi(3); // Hace que el icono de WiFi parpadee 3 veces
      }
    } else {
      Serial.print("Error en la solicitud: ");
      Serial.println(http.errorToString(httpCode));
      parpadearFallaIconoWiFi(5);
    }

    http.end(); // Cierra la conexión
  } else {
    Serial.println("No conectado a WiFi");
    pantalla_falla("Reconectando");
    pantallaPrincipal = false;
  }
}

void TakeTemp(String macAddresses[], int numAddresses) {
  // Iniciar escaneo BLE
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.println("Escaneo hecho!");
    
  tft.setCursor(3, 80); // Ajusta las coordenadas según sea necesario
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.print("Temp:    ");
  tft.print(temperature);
  tft.print("  °C");

  temperature = "";

  for (int i = 0; i < foundDevices.getCount(); i++) {
    BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);

    for (int j = 0; j < numAddresses; j++) {
      if (String(advertisedDevice.getAddress().toString().c_str()) == macAddresses[j]) {
        if (advertisedDevice.haveServiceData()) {
          std::string serviceData = advertisedDevice.getServiceData();
          uint8_t* pData = (uint8_t*)serviceData.data();
          String hexStr;
          for (int k = 0; k < serviceData.length(); k++) {
            char buf[3];
            sprintf(buf, "%02X", pData[k]);
            hexStr += buf;
          }

          if (hexStr.length() >= 10) {
            // Extracción de la temperatura
          String tempChars = hexStr.substring(7, 10); // Ajustar según la posición correcta de los datos de temperatura
          int16_t tempValue = strtol(tempChars.c_str(), NULL, 16);
          // Verificar si el valor es negativo
          if (tempValue & 0x800) { // Asumiendo que los datos son 12-bit, ajustar la máscara según el tamaño real de los datos
              tempValue = tempValue | ~0xFFF; // Extender el signo para un entero de 16 bits
          }  
            float tempResult = tempValue / 10.0;
            temperature += String(tempResult);
            temperature += ",";
          }
        }
        break;
      }
    }
  }
  temperature = temperature.substring(0, temperature.length() - 1);
  // Imprimir temperatura y humedad
  Serial.print("Temperatura: ");
  Serial.println(temperature);

  // Imprimir en la pantalla TFT
  tft.setCursor(3, 80); // Ajusta las coordenadas según sea necesario
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print("Temp:    ");
  tft.print(temperature);
  tft.print("  °C");
    enviarDatos(temperature, humidity);
  Serial.print("el Payload es:");
  Serial.print(payloadStr);
}

void TakeHumidity(String humidityMacAddresses[], int numHumidityAddresses) {
  // Iniciar escaneo BLE
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.println("Escaneo de humedad hecho!");
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);

  tft.setCursor(4, 100); // Ajusta las coordenadas para la humedad
  tft.print("Hum:     ");
  tft.print(humidity);
  tft.print("  %");

  humidity = ""; // Asegúrate de restablecer la variable de humedad

  for (int i = 0; i < foundDevices.getCount(); i++) {
    BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);

    for (int j = 0; j < numHumidityAddresses; j++) {
      if (String(advertisedDevice.getAddress().toString().c_str()) == humidityMacAddresses[j]) {
        if (advertisedDevice.haveServiceData()) {
          std::string serviceData = advertisedDevice.getServiceData();
          uint8_t* pData = (uint8_t*)serviceData.data();
          String hexStr;
          for (int k = 0; k < serviceData.length(); k++) {
            char buf[3];
            sprintf(buf, "%02X", pData[k]);
            hexStr += buf;
          }

          // Aquí necesitas ajustar la lógica para extraer la humedad
          // basándote en cómo el dispositivo codifica esos datos
          // Por ejemplo, si la humedad se codifica en los primeros dos bytes:
          if (hexStr.length() >= 10) {
              // Extracción de la humedad
            String humChars = hexStr.substring(11, 14); // Modificado para capturar los caracteres correctos
            long humValue = strtol(humChars.c_str(), NULL, 16);
            float humResult = humValue / 10.0;
            humidity += String(humResult);
            humidity += ",";
          }
        }
        break;
      }
    }
  }
  humidity = humidity.substring(0, humidity.length() - 1);
  // Imprimir humedad
  Serial.print("Humedad: ");
  Serial.print(humidity);
  Serial.println("%");

  tft.setCursor(4, 100); // Ajusta las coordenadas para la humedad
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print("Hum:     ");
  tft.print(humidity);
  tft.print("  %");

}

void TakeTempTask(void * parameter) {
  for (;;) {
    if(WiFi.status() == WL_CONNECTED) {
      if (pantallaPrincipal == false) {
          pantalla_principal();
          pantallaPrincipal = true;
      }
      TakeTemp(macAddresses, numberOfDevices); // Llama primero a la función para tomar la temperatura
      //TakeHumidity(humidityMacAddresses, numberOfHumidityDevices); // Luego llama a la función para tomar la humedad
    } else {
       
       Serial.println("Reintentando conexión wifi...");
    }
  vTaskDelay(10000 / portTICK_PERIOD_MS); // Ajusta el tiempo de espera según sea necesario
  }
}


void setup() {  
  pinMode(buttonPin, INPUT);

  Serial.begin(115200);
  tft.initR(INITR_BLACKTAB);   // Inicializa la pantalla TFT
  tft.setRotation(1);          // Establece la orientación de la pantalla

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  WiFi.mode(WIFI_STA);
  pantalla_falla("Conecte a red");
  // Intenta conectar automáticamente con las credenciales guardadas
  if (!wiFiManager.autoConnect("ControlWareTempHumiPro")) {
    Serial.println("Failed to connect and hit timeout");
  }
  tft.setFont(&FreeSansBold9pt7b);
  // Si se conecta exitosamente, imprime la dirección IP
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  // Crear tareas para FreeRTOS
  //xTaskCreatePinnedToCore(ButtonWifiTask, "ButtonWifiTask", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TakeTempTask, "TakeTempTask", 10000, NULL, 1, NULL, 1);
  
  wiFiManager.setWiFiAutoReconnect(true);
}

void loop() {}