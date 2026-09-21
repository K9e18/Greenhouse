#include <config.h>

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <DHT.h>

#include <SoftSPIB.h>
#include <LiquidCrystal_AIP31068_I2C.h>
LiquidCrystal_AIP31068_I2C lcd(0x3E, 16, 2);
RTC_DS1307 rtc; 
DateTime now;   

#define DHTTYPE DHT22 // DHT 22
DHT dht(DHT_PIN, DHTTYPE); // Initialize DHT

int OPTION = 1; 

bool last_Plus_Button = LOW;
bool current_Plus_Button = LOW;

bool last_Minus_Button = LOW;
bool current_Minus_Button = LOW;

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

#define BUZZER_pin 3

void buzz_pic(int times, int period) {
    static uint32_t buzzer_tmr;
    for (int i=0; i < times; i++) {
        if (millis() - buzzer_tmr >= period) {
            buzzer_tmr = millis();
            digitalWrite(BUZZER_pin, HIGH);
        }
        digitalWrite(BUZZER_pin, LOW);
    }
}

int max_temperature(int current_temperature) {
    int max_temperature = dht.readTemperature();
    if (max_temperature > temperature) {
        return max_temperature;
    }
    return current_temperature;
}

int min_temperature(int current_temperature) {
    int min_temperature = dht.readTemperature();
    if (min_temperature < current_temperature) {
        return min_temperature;
    }
    return current_temperature;
}

void printTwoDigits(int number) {
    if (number < 10) {
        lcd.print("0");
    }
    lcd.print(number);
}

float global_Max_temperature = 999.0;
float global_Min_temperature = -999.0;

void update_Max_Min_temperature(float current_temperature) {
    if (current_temperature > MAX_temperature) {
        MAX_temperature = current_temperature;
    }
    if (current_temperature < MIN_temperature) {
        MIN_temperature = current_temperature;
    }
}

int global_Max_humidity = 100;
int global_Min_humidity = 0;

void update_Max_Min_humidity(int current_humidity) {
    if (current_humidity > MAX_humidity) {
        MAX_humidity = current_humidity;
    }
    if (current_humidity < global_MIN_humidity) {
        MIN_humidity = current_humidity;
    }
}

void setup() {
    dht.begin();
    
    lcd.init();
//    lcd.backlight();
    
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

    int current_humidity = dht.readHumidity();
    float current_temperature = dht.readTemperature();

    static uint32_t dht_tmr;
    if (millis() - dht_tmr >= DHT_check_period) {
        dht_tmr = millis(); 
        
        if (!isnan(current_temperature)) {
            update_Max_Min_temperature(current_temperature);
            update_Max_Min_humidity(current_humidity);
        }
        current_humidity = dht.readHumidity();
        current_temperature = dht.readTemperature();
    }

    switch (OPTION) {
        case 1:
            lcd.setCursor(0, 0);
            printTwoDigits(now.hour());
            lcd.print(":");
            printTwoDigits(now.minute()); 

            // print temperature
            lcd.setCursor(10, 0);
            lcd.print(temperature);
            lcd.print("C   ");
            
            // print humidity
            lcd.setCursor(10, 1);
            lcd.print(humidity);
            lcd.print("%    ");

            break;

        case 2:
            // print max end min temperature
            // print max end min moisture
        case 3:
            // mode for start watering
        case 4:
            // mode for start faning
        case 5:
            // mode for show stats
    }


    if (now.hour() == WATERING_HOUR && now.minute() == WATERING_MIN && !isWateredToday) {
        lcd.setCursor(11, 1);
        lcd.print("PUMP "); 
        digitalWrite(PUMP_PIN, HIGH); 
        bool isWatering = digitalRead(PUMP_PIN);
        if (isWatering) {
            analogWrite(RED_pin, 0);
            analogWrite(GREEN_pin, 0);
            analogWrite(BLUE_pin, 255);

            buzz_pic(2, 25);
        }
        else if (isWatering == false) {
            analogWrite(RED_pin, 0);
            analogWrite(GREEN_pin, 0);
            analogWrite(BLUE_pin, 0);
        }
        
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
            OPTION = MIN_OPTION;
        }
    }
    last_Minus_Button = current_Minus_Button; 

    lcd.setCursor(0, 1);
    lcd.print("Opt: ");
    lcd.print(OPTION);
}
