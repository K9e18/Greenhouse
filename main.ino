#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define HEATER_PIN 5
#define FAN_PIN 4
#define PUMP_PIN 2

#define TMP_SENSOR_PIN 0 // temperature sensor pin
#define SPEAKER_PIN 3

// Display
/*
GND display -> GND arduino
VCC display -> VCC arduino
SDA display -> A4
SCL display -> A5
*/

/*
Time module
GND -> GND arduino
VCC -> VCC arduino
SDA -> A4
SCL -> A5
*/

#define MAX_OPTION
#define MIN_OPTION 1

// RGB LED
#define RED_pin 1
#define GREEN_pin 2
#define BLUE_pin 3

#define Plus_Button_Pin 7
bool last_Plus_Button = LOW;
bool current_Plus_Button = LOW;

#define Minus_Button_Pin 8
bool last_Minus_Button = LOW;
bool current_Minus_Button = LOW;

#define Mode_Button_Pin 9
bool last_Mode_Button = LOW;
bool current_Mode_Button = LOW;

// Greenhouse settings
#define WATERING_HOUR
#define WATERING_MIN 0 // minute of watering
#define WATERING_DURATION
uint32_t PUMP_tmr;

bool isWateredToday = false;

bool debounce(bool last, short PIN) {
    bool current = digitalRead(PIN);
    if (last != current) {
        uint32_t tmr;
        if(millis() - tmr >= 5) {
            current = digitalRead(PIN);
            tmr = millis();
        }
    }
    return current;
}

void printTwoDigits(int number) {
    if (number < 10) {
        lcd.print("0");
    }
    lcd.print(number);
}

void setup() {
    // display initialiazation
    lcd.init();
    lcd.backlight();
    if (!rtc.begin()) {
        lcd.setCursor(0, 0);
        lcd.print("RTC Error!");
        while(1);
    }

    // if clock losed power, set compilation time
    if (!rtc.isrunnin()) {
        rtc.adjust(DateTime(f(__DATE__), F(__TIME__));
    }

    pinMode(HEATER_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);

    digitalWrite(PUMP_PIN, LOW);
}

void loop() {
    // printing time
    lcd.setCursor(0, 0);
    printTwoDigits(now.hour());
    lcd.print(":");
    printTwoDigits(now.min());
    lcd.print(":");
    printTwoDigits(now.sec());

    if (now.hour() == WATERING_HOUR && now.min() == WATERING_MIN && !isWateredToday) {
        lcd.setCursor(11, 1);
        lcd.print("PUMP "); // write on display that watering is running
        digitalWrite(PUMP_PIN, LOW);
        if (millis() - PUMP_tmr >= WATERING_DURATION) {
            digitalWrite(PUMP_PIN, LOW);
            PUMP_tmr = millis();
        }

        isWateredToday = true; // today it already watered
        lcd.setCursor(11, 1);
        lcd.print("     ");
    }

    if (now.min() != WATERING_MIN) {
        isWateredToday = false;
    }

    current_Mode_Button = debounce(last_Mode_Button, Mode_Button_Pin);

    current_Plus_Button = debounce(last_Plus_Button, Plus_Button_Pin);
    if (last_Plus_Button == LOW &&  current_Plus_Button == HIGH) {
        OPTION++;
        if (OPTION > MAX_OPTION) {
            OPTION = MAX_OPTION;
        }
    }
    last_Plus_Button = current_Plus_Button;

    current_Minus_Button = debounce(last_Minus_Button, Minus_Button_Pin);
    if (last_Minus_Button == LOW && current_Minus_Button == HIGH) {
        OPTION--;
        if (OPTION < MIN_OPTION) {
            OPTION = MIN_OPTION;
        }
    }

    switch (MODE) {
        case 1:
            
}
