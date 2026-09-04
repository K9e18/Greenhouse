#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc; 
DateTime now;   

#define HEATER_PIN 5
#define FAN_PIN 4
#define PUMP_PIN 2
#define TMP_SENSOR_PIN 0 
#define SPEAKER_PIN 3

#define DHT_check_period 2000

#define MAX_OPTION 5
#define MIN_OPTION 1
int OPTION = 1; 

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

#define WATERING_HOUR 14
#define WATERING_MIN 0 
#define WATERING_DURATION 5000 
uint32_t PUMP_tmr;            

bool isWateredToday = false;

bool debounce(bool last, short PIN) {
    bool current = digitalRead(PIN);
    if (last != current) {
        static uint32_t tmr; 
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
    lcd.init();
    lcd.backlight();
    
    if (!rtc.begin()) {
        lcd.setCursor(0, 0);
        lcd.print("RTC Error!");
        while(1);
    }

    if (!rtc.isrunning()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    pinMode(HEATER_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);

    digitalWrite(PUMP_PIN, LOW);
}

void loop() {
    now = rtc.now(); 

    lcd.setCursor(0, 0);
    printTwoDigits(now.hour());
    lcd.print(":");
    printTwoDigits(now.minute()); 
    lcd.print(":");
    printTwoDigits(now.second()); 

    if (now.hour() == WATERING_HOUR && now.minute() == WATERING_MIN && !isWateredToday) {
        lcd.setCursor(11, 1);
        lcd.print("PUMP "); 
        digitalWrite(PUMP_PIN, HIGH); 
        
        if (millis() - PUMP_tmr >= WATERING_DURATION) {
            digitalWrite(PUMP_PIN, LOW);
            PUMP_tmr = millis();
            isWateredToday = true; 
            lcd.setCursor(11, 1);
            lcd.print("     ");
        }
    }

    if (now.minute() != WATERING_MIN) {
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
    last_Minus_Button = current_Minus_Button; 

    // Выводим выбранную опцию на экран (строка 2, позиция 0)
    lcd.setCursor(0, 1);
    lcd.print("Opt: ");
    lcd.print(OPTION);

    static uint32_t dht_tmr;
    if (millis() - dht_tmr >= DHT_check_period) {
        dht_tmr = millis(); 
        
        int temperature = 24; 
        int humidity = 50;

        lcd.setCursor(10, 0);
        lcd.print(temperature);
        lcd.print("C   ");
    }
}

