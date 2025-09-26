#include <Arduino.h>

#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define COLUMNS 16
#define LINES 2
#define I2C_ADDRESS 0x27 

#define DHTTYPE DHT22
#define DHTPIN 13

#define LDR_ANALOG_PORT 34

const float LOW_LUMINOSITY = 15;
const float MEDIUM_LUMINOSITY = 33;

const float LOW_TEMPERATURE = 10;
const float MEDIUM_TEMPERATURE = 15;

const float LOW_HUMIDITY = 50;
const float MEDIUM_HUMIDITY = 70;

const long applicationDelay = 1000;
const long displayIntervalDuration = 5000;

bool changeOk = true;

int currentIndex = 0;
long displayTime = 0;

float *luminosityHistory;
float *temperatureHistory;
float *humidityHistory;

int historySize;

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(I2C_ADDRESS, COLUMNS, LINES);

float avarage(float *array)
{
	float sum = 0;
	for (int i = 0; i < historySize; i++)
	{
		sum += array[i];
	}
	return (sum / historySize);
}

void lcdPrintText(const char *text, int column, int line)
{
	lcd.setCursor(column, line);
	lcd.print(text);
}

void lcdWriteIcon(int emote)
{
	lcd.write(byte(emote));
}

void lcdPrintStatusText(const char *text, const char *prefix, float value, int decimalPlaces, const char *sufix)
{
	lcd.clear();

	lcdPrintText(text, 0, 0);

	lcd.setCursor(0, 1);
	lcd.print(prefix);
	lcd.print(value, decimalPlaces);
	lcd.print(sufix);
}

void lcdPrintStatusWithIcon(const char *text, const char *prefix, float value, int decimalPlaces, int emote)
{
	lcd.clear();

	lcdPrintText(text, 0, 0);

	lcd.setCursor(0, 1);
	lcd.print(prefix);
	lcd.print(value, decimalPlaces);
	lcdWriteIcon(emote);
}

void lcdPrintStatus()
{
	float luminosityAvarage = avarage(luminosityHistory);
	float humidityAvarage = avarage(humidityHistory);
	float temperatureAvarage = avarage(temperatureHistory);

	Serial.println("*******************");
	Serial.print("Luminosity Avarage: ");
	Serial.println(luminosityAvarage, 0);
	Serial.print("Humidity Avarage: ");
	Serial.println(humidityAvarage, 0);
	Serial.print("Temperature Avarage: ");
	Serial.println(temperatureAvarage, 1);
	Serial.println("*******************");

	bool isLuminosityAvarageHigh = (luminosityAvarage > MEDIUM_LUMINOSITY);
	bool isHumidityAvarageHigh = (humidityAvarage > MEDIUM_HUMIDITY);
	bool isTemperatureAvarageHigh = (temperatureAvarage > MEDIUM_TEMPERATURE);

	bool isLuminosityAvarageMedium = (luminosityAvarage < MEDIUM_LUMINOSITY && luminosityAvarage >= LOW_LUMINOSITY);
	bool isHumidityAvarageLow = (humidityAvarage < LOW_HUMIDITY);
	bool isTemperatureAvarageLow = (temperatureAvarage < LOW_TEMPERATURE);

	bool isHumidityOk = (!isHumidityAvarageLow && !isHumidityAvarageHigh) && (changeOk);
	bool isTemperatureOk = (!isTemperatureAvarageLow && !isTemperatureAvarageHigh);

	lcd.clear();
	if (isLuminosityAvarageHigh)
	{
		lcdPrintText("Ambiente muito", 0, 0);
		lcdPrintText("CLARO", 0, 1);
	}
	else if (isHumidityAvarageHigh)
	{
		lcdPrintStatusText("Umidade ALTA", "Umidade = ", humidityAvarage, 0, "%");
	}
	else if (isTemperatureAvarageHigh)
	{
		lcdPrintStatusWithIcon("Temp. ALTA", "Temp. = ", temperatureAvarage, 1, 15);
	}
	else if (isLuminosityAvarageMedium)
	{
		lcdPrintText("Ambiente a meia", 0, 0);
		lcdPrintText("luz", 0, 1);
	}
	else if (isHumidityAvarageLow)
	{
		lcdPrintStatusText("Umidade BAIXA", "Umidade = ", humidityAvarage, 0, "%");
	}
	else if (isTemperatureAvarageLow)
	{
		lcdPrintStatusWithIcon("Temp. BAIXA", "Temp. = ", temperatureAvarage, 1, 15);
	}
	else if (isHumidityOk)
	{
		lcdPrintStatusText("Umidade OK", "Umidade = ", humidityAvarage, 0, "%");
		changeOk = false;
	}
	else if (isTemperatureOk)
	{
		lcdPrintStatusWithIcon("Temp. OK", "Temp. = ", temperatureAvarage, 1, 15);
		changeOk = true;
	}
	else
	{
		lcdPrintText("Ambiente OK", 0, 0);
		lcdPrintText("Normal", 0, 1);
	}

	currentIndex = 0;
}

void lcdInit()
{
	lcd.init();
	lcd.backlight();
	lcd.clear();

	byte shit[8] = {
		B00000,
		B10001,
		B00000,
		B00000,
		B01110,
		B10001,
		B00000,
	};

	byte smile[8] = {
		B00000,
		B10001,
		B00000,
		B00000,
		B10001,
		B01110,
		B00000,
	};

	byte serious[8] = {
		B00000,
		B10001,
		B00000,
		B00000,
		B11111,
		B00000,
		B00000,
	};

	byte wow[8] = {
		B00000,
		B10001,
		B00000,
		B01110,
		B10001,
		B10001,
		B01110,
	};

	byte lowLight[8] = {
		B00100,
		B01110,
		B00000,
		B00100,
		B01110,
		B00000,
		B00000,
		B00000,
	};

	byte mediumLight[8] = {
		B00100,
		B01110,
		B00000,
		B00100,
		B01110,
		B11111,
		B11111,
		B00000,
	};

	byte fullLight[8] = {
		B00100,
		B01110,
		B00000,
		B00100,
		B01110,
		B11111,
		B11111,
		B11111,
	};

	byte lowTemp[8] = {
		B00100,
		B00100,
		B00100,
		B00100,
		B00100,
		B00000,
		B00100,
		B00000};

	byte mediumTemp[8] = {
		B00100,
		B00100,
		B00100,
		B00100,
		B00100,
		B00000,
		B01110,
		B00000};

	byte highTemp[8] = {
		B00100,
		B00100,
		B00100,
		B00100,
		B00100,
		B00000,
		B11111,
		B00000};

	byte lowHumidity[8] = {
		B00100,
		B00100,
		B00000,
		B00000,
		B00000,
		B00000,
		B00100,
		B00000};

	byte mediumHumidity[8] = {
		B00100,
		B01110,
		B00100,
		B00000,
		B00000,
		B00000,
		B01110,
		B00000};

	byte highHumidity[8] = {
		B00100,
		B01110,
		B11111,
		B00000,
		B00000,
		B00000,
		B11111,
		B00000};

	byte celsiusOne[8] = {
		B01000,
		B10100,
		B01010,
		B00101,
		B00100,
		B00100,
		B00101,
		B00010};

	byte celsiusTwo[8] = {
		B10000,
		B00110,
		B01001,
		B01000,
		B01000,
		B01000,
		B01001,
		B00110};

	byte celsiusThree[8] = {
		B10000,
		B01110,
		B10001,
		B10000,
		B10000,
		B10001,
		B01110,
		B00000};

	lcd.createChar(0, shit);
	lcd.createChar(1, smile);
	lcd.createChar(3, serious);
	lcd.createChar(4, wow);

	lcd.createChar(5, lowLight);
	lcd.createChar(6, mediumLight);
	lcd.createChar(7, fullLight);

	lcd.createChar(8, lowTemp);
	lcd.createChar(9, mediumTemp);
	lcd.createChar(10, highTemp);

	lcd.createChar(11, lowHumidity);
	lcd.createChar(12, mediumHumidity);
	lcd.createChar(13, highHumidity);

	lcd.createChar(14, celsiusTwo);
	lcd.createChar(15, celsiusThree);
}

float getLuminosityPercentage()
{
	float ldrValue = analogRead(LDR_ANALOG_PORT);
	float luminosityPercentage = map(ldrValue, 4063, 32, 0, 100);

	Serial.print("Luminosidade: ");
	Serial.println(luminosityPercentage);

	return luminosityPercentage;
}

float getLuxValue()
{
	const float GAMMA = 0.7;
	const float RL10 = 50;
	float voltage = analogRead(LDR_ANALOG_PORT) / 1024. * 5;
	Serial.print(analogRead(LDR_ANALOG_PORT));
	float resistance = 2000 * voltage / (1 - voltage / 5);
	float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));

	Serial.print("Lux: ");
	Serial.println(lux);

	return lux;
}

long getHumidityPercentage()
{
	float humidityPercentage = dht.readHumidity();

	if (isnan(humidityPercentage))
	{
		Serial.print("Could not get the temperature in DHT");
		return 0;
	}

	Serial.print("Humidade: ");
	Serial.println(humidityPercentage);

	return humidityPercentage;
}

double getTemperatureCelsius()
{
	double tempCelsius = dht.readTemperature();

	if (isnan(tempCelsius))
	{
		Serial.print("Could not get the temperature in DHT");
		return 0;
	}

	Serial.print("Temperatura: ");
	Serial.println(tempCelsius);

	return tempCelsius;
}

void setup()
{
	Serial.begin(115200);

	lcdInit();
	lcdPrintText("Teste", 0, 0);

	historySize = (displayIntervalDuration / applicationDelay);

	luminosityHistory = (float *)malloc(historySize * sizeof(float));
	temperatureHistory = (float *)malloc(historySize * sizeof(float));
	humidityHistory = (float *)malloc(historySize * sizeof(float));

	if (!luminosityHistory || !temperatureHistory || !humidityHistory)
	{
		Serial.print("Error allocating memory.");
		while (true)
			;
	}

	dht.begin();
}

void loop()
{
	unsigned long currentTime = millis();

	Serial.println("==================");
	float luminosity = getLuminosityPercentage();
	long humidity = getHumidityPercentage();
	double temperature = getTemperatureCelsius();
	Serial.println("==================");

	luminosityHistory[currentIndex] = luminosity;
	humidityHistory[currentIndex] = humidity;
	temperatureHistory[currentIndex] = temperature;

	currentIndex = (currentIndex + 1);

	if ((currentTime - displayTime >= displayIntervalDuration))
	{
		lcdPrintStatus();
		displayTime = currentTime;
	}

	delay(applicationDelay);
}
