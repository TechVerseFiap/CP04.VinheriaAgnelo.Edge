#include <Arduino.h>

#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi
char SSID[] = "Wokwi-GUEST";
char PASSWORD[] = "";

WiFiClient espClient;

// MQTT
#define TOPIC_SUBSCRIBE "/TEF/testxxx/cmd"
#define TOPIC_PUBLISH_0 "/TEF/testxxx/attrs"
#define TOPIC_PUBLISH_1 "/TEF/testxxx/attrs/luminosity"
#define TOPIC_PUBLISH_2 "/TEF/testxxx/attrs/temperature"
#define TOPIC_PUBLISH_3 "/TEF/testxxx/attrs/humidity"

#define ID_MQTT "fiware_xxx"

const char *BROKER_MQTT = "broker.emqx.io";
const int BROKER_PORT = 1883;

PubSubClient MQTT(espClient);

// LCD I2C
#define COLUMNS 16
#define LINES 2
#define I2C_ADDRESS 0x27

LiquidCrystal_I2C lcd(I2C_ADDRESS, COLUMNS, LINES);

// DHT
#define DHTTYPE DHT22
#define DHTPIN 13

DHT dht(DHTPIN, DHTTYPE);

// LDR
#define LDR_ANALOG_PORT 34

// Application
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
		lcdPrintStatusWithIcon("Temp. ALTA", "Temp. = ", temperatureAvarage, 1, 0);
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
		lcdPrintStatusWithIcon("Temp. BAIXA", "Temp. = ", temperatureAvarage, 1, 0);
	}
	else if (isHumidityOk)
	{
		lcdPrintStatusText("Umidade OK", "Umidade = ", humidityAvarage, 0, "%");
		changeOk = false;
	}
	else if (isTemperatureOk)
	{
		lcdPrintStatusWithIcon("Temp. OK", "Temp. = ", temperatureAvarage, 1, 0);
		changeOk = true;
	}
	else
	{
		lcdPrintText("Ambiente OK", 0, 0);
		lcdPrintText("Normal", 0, 1);
	}

	currentIndex = 0;
}

void lcdBegin()
{
	lcd.init();
	lcd.backlight();
	lcd.clear();

	byte celsiusChar[8] = {
		B10000,
		B01110,
		B10001,
		B10000,
		B10000,
		B10001,
		B01110,
		B00000
	};

	lcd.createChar(0, celsiusChar);
}

float getLuminosityPercentage()
{
	float ldrValue = analogRead(LDR_ANALOG_PORT);
	float luminosityPercentage = map(ldrValue, 4063, 32, 0, 100);

	Serial.print("Luminosidade: ");
	Serial.println(luminosityPercentage);

	return luminosityPercentage;
}

float getHumidityPercentage()
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

float getTemperatureCelsius()
{
	float tempCelsius = dht.readTemperature();

	if (isnan(tempCelsius))
	{
		Serial.print("Could not get the temperature in DHT");
		return 0;
	}

	Serial.print("Temperatura: ");
	Serial.println(tempCelsius);

	return tempCelsius;
}

void mqttBegin()
{
	MQTT.setServer(BROKER_MQTT, BROKER_PORT);
}

void reconnectMQTT()
{
	while (!MQTT.connected())
	{
		Serial.print("Trying to connect with the MQTT Broker: ");
		Serial.println(BROKER_MQTT);
		if (MQTT.connect(ID_MQTT))
		{
			Serial.println("Connected successfully with Broker");
			MQTT.subscribe(TOPIC_SUBSCRIBE);
			MQTT.publish(TOPIC_PUBLISH_0, "teste");
		}
		else
		{
			Serial.println("Failed trying to connect with Broker");
			delay(2000);
		}
	}
}

void reconnectWiFi()
{
	if (WiFi.status() == WL_CONNECTED)
	{
		return;
	}

	WiFi.begin(SSID, PASSWORD);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(100);
		Serial.print(".");
	}

	Serial.println();
	Serial.print("Connected with success into the net ");
	Serial.println(SSID);
	Serial.println("IP: ");
	Serial.println(WiFi.localIP());
}

void verifyConnectionWiFiAndMQTT()
{
	reconnectWiFi();
	if (!MQTT.connected())
		reconnectMQTT();
}

void wifiBegin()
{
	delay(10);
	Serial.println("WiFi Connection");
	Serial.print("Connecting into the net: ");
	Serial.println(SSID);
	Serial.println("Wait");

	reconnectWiFi();
}

void setup()
{
	Serial.begin(115200);
	lcdBegin();
	wifiBegin();
	mqttBegin();
	dht.begin();

	historySize = (displayIntervalDuration / applicationDelay);

	luminosityHistory = (float *)malloc(historySize * sizeof(float));
	temperatureHistory = (float *)malloc(historySize * sizeof(float));
	humidityHistory = (float *)malloc(historySize * sizeof(float));

	if (!luminosityHistory || !temperatureHistory || !humidityHistory)
	{
		Serial.print("Error allocating memory.");
		while (true);
	}
}

void loop()
{
	verifyConnectionWiFiAndMQTT();

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

	MQTT.loop();
}