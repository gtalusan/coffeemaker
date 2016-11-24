#include <LowPower.h>

static const int BUTTON_PIN = 2;
static const int LED_PIN = A1;
static const int RELAY_PIN = A0;

enum COFFEE_MAKER_STATE { OFF = 0, ON };
static volatile COFFEE_MAKER_STATE coffeeMaker = OFF;
static volatile unsigned long startTimeMs = 0;
static volatile unsigned long lastISR = 0;
static volatile unsigned long duty = 0;

static void buttonISR()
{
	static const unsigned long BUTTON_DEBOUNCE_TIME_MS = 200L;
	unsigned long now = millis();
	if (now - lastISR < BUTTON_DEBOUNCE_TIME_MS) {
		return;
	}
	lastISR = now;
	coffeeMaker = coffeeMaker == OFF? ON : OFF;
	if (coffeeMaker == ON) {
		startTimeMs = now;
		duty = now;
		analogWrite(LED_PIN, 128);
	}
}

void setup()
{
	attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, RISING);
	pinMode(LED_PIN, OUTPUT);
	pinMode(RELAY_PIN, OUTPUT);
	pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
	static const unsigned long DUTY_ON_MS = (45L * 1000);
	static const unsigned long DUTY_OFF_MS = (30L * 1000) + DUTY_ON_MS;
	static const unsigned long BREW_TIME_MS = 10L * 60 * 1000;
	unsigned long now = millis();
	if (coffeeMaker == ON) {
		unsigned long duration = now - duty;
		if (duration < DUTY_ON_MS) {
			analogWrite(RELAY_PIN, 128);
		}
		else if (duration < DUTY_OFF_MS) {
			analogWrite(RELAY_PIN, 0);
		}
		else {
			duty = now;
		}
		if (now - startTimeMs >= BREW_TIME_MS) {
			coffeeMaker = OFF;
		}
	}
	static const unsigned long IDLE_TIME_MS = 500L;
	if (coffeeMaker == OFF && now - lastISR >= IDLE_TIME_MS) {
		analogWrite(LED_PIN, 0);
		analogWrite(RELAY_PIN, 0);
		LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
	}
}
