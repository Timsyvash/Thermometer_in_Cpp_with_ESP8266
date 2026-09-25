#include <Adafruit_NeoPixel.h>
#include <DHT.h>

// --- Pin Definitions ---
#define DHTPIN D4       // GPIO2 — пін датчика DHT11
#define NEOPIXEL_PIN D1 // GPIO5 — пін матриці WS2812B (використовуємо D1!)

// --- Configuration ---
#define DHTTYPE DHT11
#define NUMPIXELS 64 // 64 світлодіоди (матриця 8x8)

DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Шрифт 3x5 для цифр 0-9
const uint8_t digits[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
    {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
    {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
    {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
    {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
    {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
    {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
    {0b111, 0b001, 0b001, 0b010, 0b010}, // 7
    {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
    {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
};

// Формула перерахунку координат
int getPixelIndex(int x, int y)
{
  if (x < 0 || x > 7 || y < 0 || y > 7)
    return -1;

  // Пряма адресація рядків
  return y * 8 + x;
}

void drawDigit(int digit, int offsetX, uint32_t color)
{
  if (digit < 0 || digit > 9)
    return;

  for (int row = 0; row < 5; row++)
  {
    for (int col = 0; col < 3; col++)
    {
      if ((digits[digit][row] >> (2 - col)) & 1)
      {
        int px = offsetX + col;
        int py = row + 1;

        int idx = getPixelIndex(px, py);
        if (idx >= 0)
          pixels.setPixelColor(idx, color);
      }
    }
  }
}

void drawDegreeSymbol(uint32_t color)
{
  int idx = getPixelIndex(7, 1);
  if (idx >= 0)
    pixels.setPixelColor(idx, color);
}

void drawPercentSymbol(uint32_t color)
{
  int p1 = getPixelIndex(7, 1);
  int p2 = getPixelIndex(7, 5);

  if (p1 >= 0)
    pixels.setPixelColor(p1, color);
  if (p2 >= 0)
    pixels.setPixelColor(p2, color);
}

void displayValue(int value, bool isTemperature)
{
  pixels.clear();

  value = constrain(value, 0, 99);
  int d1 = value / 10;
  int d2 = value % 10;

  uint32_t numColor = isTemperature ? pixels.Color(255, 0, 0) : pixels.Color(255, 0, 0);
  uint32_t symColor = pixels.Color(255, 0, 0);

  drawDigit(d1, 0, numColor);
  drawDigit(d2, 4, numColor);

  if (isTemperature)
    drawDegreeSymbol(symColor);
  else
    drawPercentSymbol(symColor);

  pixels.show();
}

void signalError()
{
  pixels.clear();
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(50, 0, 50));
  }
  pixels.show();
}

unsigned long lastSensorRead = 0;
unsigned long lastDisplaySwitch = 0;
bool showTemp = true;

float humidity = 0;
float temperature = 0;
bool sensorError = false;

void setup()
{
  Serial.begin(115200);
  dht.begin();
  pixels.begin();
  pixels.setBrightness(5);
  pixels.clear();
  pixels.show();
}

void loop()
{
  unsigned long currentMillis = millis();

  // Опитування датчика кожні 2 секунди
  if (currentMillis - lastSensorRead >= 2000)
  {
    lastSensorRead = currentMillis;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
      sensorError = true;
      Serial.println("DHT11 Read Error!");
    }
    else
    {
      sensorError = false;
      humidity = h;
      temperature = t;
      Serial.printf("Temp: %.1f C | Hum: %.1f %%\n", temperature, humidity);
    }
  }

  // Перемикання показників кожні 3 секунди
  if (currentMillis - lastDisplaySwitch >= 3000)
  {
    lastDisplaySwitch = currentMillis;
    showTemp = !showTemp;
  }

  // Оновлення екрана
  if (sensorError)
  {
    signalError();
  }
  else
  {
    if (showTemp)
      displayValue((int)temperature, true);
    else
      displayValue((int)humidity, false);
  }

  yield();
}