#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6 //DatenPin D6 am Arduino 
#define NUMPIXELS 11 //Anzahl der LEDs 
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

SoftwareSerial co2Serial(2, 3); // define MH-Z19 RX TX

bool preHeatReady = false;
int counter = 0;

// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(9600);
	Serial.println("CO2-Ufo by M. Albrecht 2021");

	pixels.begin();
	for (int i = 0; i < NUMPIXELS; i++)
	{
		pixels.setPixelColor(i, pixels.Color(200 - (10*i), 10*1, 100+5*i));
		pixels.show();
		delay(100);
	}

	co2Serial.begin(9600);
	setSensorABC();
}


void loop()
{
	int ppm, temperature = 0;

	readSensor(&ppm, &temperature);

	if (preHeatReady == false)
	{
		counter++;
		Serial.print("PPM: ");
		Serial.print(ppm);
		Serial.print(" Temperature: ");
		Serial.print(temperature);
		Serial.print(" Counter-PreHeat: ");
		Serial.println(counter);

		if (((ppm != 550) && (counter > 20)) || (counter > 300))
		{
			preHeatReady = true;
		}

		if (counter % 2 == 0)
		{
			SetLed(0, 77, 102);
		}
		else
		{
			SetLed(0, 0, 0);
		}

		delay(1000);
	}
	else
	{
		Serial.print("PPM: ");
		Serial.print(ppm);
		Serial.print(" Temperature: ");
		Serial.println(temperature);

		if (ppm < 700) //grün
		{
			SetLed(0, 102, 0);
		}
		else if (ppm < 1000) //gelb
		{
			SetLed(102, 102, 0);
		}
		else if (ppm < 1300) //orange
		{
			SetLed(102, 61, 0);
		}
		else //rot
		{
			SetLed(102, 20, 0);
		}

		delay(5000);
	}
}

void SetLed(int r, int g, int b)
{
	for (int i = 0; i < NUMPIXELS; i++)
	{
		pixels.setPixelColor(i, pixels.Color(r, g, b));
	}

	pixels.show();
}

void readSensor(int* ppm, int* temperature) 
{
	byte response[9];

	byte cmd[9] = { 0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79 };
	co2Serial.write(cmd, 9);

	memset(response, 0, 9);
	while (co2Serial.available() == 0) 
	{
		delay(1000);
	}

	co2Serial.readBytes(response, 9);

	byte check = getCheckSum(response);
	if (response[8] != check) {
		Serial.println("Fehler in der Übertragung!");
		return;
	}
	*ppm = 256 * (int)response[2] + (int)response[3];
	*temperature = (int)response[4] - 40;
}

byte getCheckSum(byte* packet)
{
	byte i;
	byte checksum = 0;
	for (i = 1; i < 8; i++) {
		checksum += packet[i];
	}
	checksum = 0xff - checksum;
	checksum += 1;
	return checksum;
}

void setSensorABC()
{
	byte response[9];

	byte cmd[9] = { 0xFF,0x01,0x79,0xA0,0x00,0x00,0x00,0x00,0xe5};
	co2Serial.write(cmd, 9);
}