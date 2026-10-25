#include <config.h>
#include <symbols.h>

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <DHT.h>

#include <SoftSPIB.h>
#include <LiquidCrystal_AIP31068_I2C.h>

LiquidCrystal_AIP31068_I2C lcd(0x3E, 16, 2);
RTC_DS1307 rtc; 
DateTime now;   

#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

int OPTION = 1; 

bool last_Plus_Button = LOW;
bool current_Plus_Button = LOW;
bool last_Minus_Button = LOW;
bool current_Minus_Button = LOW;
bool last_Mode_Button = LOW;
bool current_Mode_Button = LOW;

uint32_t PUMP_tmr = 0;            
bool isWateredToday = false;

// ==========================================
//            HELPER FUNCTIONS
// ==========================================

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

void printTwoDigits(int number) {
    if (number < 10) {
        lcd.print("0");
    }
    lcd.print(number);
}

float MAX_temperature = -999.0;
float MIN_temperature = 999.0;

void update_Max_Min_temperature(float current_temperature) {
    if (current_temperature > MAX_temperature) MAX_temperature = current_temperature;
    if (current_temperature < MIN_temperature) MIN_temperature = current_temperature;
}

int MAX_humidity = 0;
int MIN_humidity = 100;

void update_Max_Min_humidity(int current_humidity) {
    if (current_humidity > MAX_humidity) MAX_humidity = current_humidity;
    if (current_humidity < MIN_humidity) MIN_humidity = current_humidity;
}

// ==========================================
//            RGB ANIMATION FUNCTIONS
// ==========================================

void RGB_watering() {
    static short light_option = 1;
    static int32_t light_flash_tmr;
    
    if (millis() - light_flash_tmr >= light_flash_period) {
        light_flash_tmr = millis();
        light_option++;
        if (light_option > MAX_light_WATERING_options) light_option = 1;
    }
    
    switch(light_option) {
        case 1:  analogWrite(RED_pin, 102); analogWrite(GREEN_pin, 178); analogWrite(BLUE_pin, 255); break;
        case 2:  analogWrite(RED_pin, 51);  analogWrite(GREEN_pin, 178); analogWrite(BLUE_pin, 255); break;
        case 3:  analogWrite(RED_pin, 0);   analogWrite(GREEN_pin, 128); analogWrite(BLUE_pin, 255); break;
        case 4:  analogWrite(RED_pin, 0);   analogWrite(GREEN_pin, 102); analogWrite(BLUE_pin, 204); break;
        case 5:  analogWrite(RED_pin, 0);   analogWrite(GREEN_pin, 76);  analogWrite(BLUE_pin, 153); break;
        case 6:  analogWrite(RED_pin, 0);   analogWrite(GREEN_pin, 51);  analogWrite(BLUE_pin, 102); break;
        case 7:  analogWrite(RED_pin, 0);   analogWrite(GREEN_pin, 25);  analogWrite(BLUE_pin, 51);  break;
        case 8:  analogWrite(RED_pin, 0);   analogWrite(GREEN_pin, 0);   analogWrite(BLUE_pin, 51);  break;
        case 9:  analogWrite(RED_pin, 0);   analogWrite(GREEN_pin, 0);   analogWrite(BLUE_pin, 102); break;
        case 10: analogWrite(RED_pin, 0);   analogWrite(GREEN_pin, 0);   analogWrite(BLUE_pin, 153); break;
        case 11: analogWrite(RED_pin, 0);   analogWrite(GREEN_pin, 0);   analogWrite(BLUE_pin, 204); break;
        case 12: analogWrite(RED_pin, 0);   analogWrite(GREEN_pin, 0);   analogWrite(BLUE_pin, 255); break;
        case 13: analogWrite(RED_pin, 51);  analogWrite(GREEN_pin, 51);  analogWrite(BLUE_pin, 255); break;
        case 14: analogWrite(RED_pin, 102); analogWrite(GREEN_pin, 102); analogWrite(BLUE_pin, 255); break;
    }
}

void RGB_heatering() {
    static short light_option = 1;
    static int32_t light_flash_tmr;
    
    if (millis() - light_flash_tmr >= light_flash_period) {
        light_flash_tmr = millis();
        light_option++;
        if (light_option > MAX_light_HEATERING_options) light_option = 1;
    }
    
    switch(light_option) {
        case 1: analogWrite(RED_pin, 136); analogWrite(GREEN_pin, 8);  analogWrite(BLUE_pin, 8);  break; 
        case 2: analogWrite(RED_pin, 170); analogWrite(GREEN_pin, 74); analogWrite(BLUE_pin, 68); break; 
        case 3: analogWrite(RED_pin, 238); analogWrite(GREEN_pin, 75); analogWrite(BLUE_pin, 43); break; 
        case 4: analogWrite(RED_pin, 165); analogWrite(GREEN_pin, 42); analogWrite(BLUE_pin, 42); break; 
        case 5: analogWrite(RED_pin, 128); analogWrite(GREEN_pin, 0);  analogWrite(BLUE_pin, 32); break; 
        case 6: analogWrite(RED_pin, 110); analogWrite(GREEN_pin, 38); analogWrite(BLUE_pin, 14); break; 
        case 7: analogWrite(RED_pin, 204); analogWrite(GREEN_pin, 85); analogWrite(BLUE_pin, 0);  break; 
    }
}

// ==========================================
//              SETUP & LOOP
// ==========================================

void setup() {
    dht.begin();
    lcd.init();
    
    if (!rtc.begin()) {
        lcd.setCursor(0, 0);
        lcd.print("RTC Error!");
        while(1);
    }

    // Initialize custom symbols
    lcd.createChar(0, degree_Symbol);
    lcd.createChar(1, fan_is_on_Symbol);
    lcd.createChar(2, water_drop_Symbol); // Water drop symbol initialized

    if (!rtc.isrunning()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    pinMode(HEATER_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(LIGHT_PANEL_PIN, OUTPUT);
    
    pinMode(Plus_Button_Pin, INPUT);
    pinMode(Minus_Button_Pin, INPUT);
    pinMode(Mode_Button_Pin, INPUT);

    digitalWrite(PUMP_PIN, LOW);
}

void loop() {
    now = rtc.now(); 

    int current_humidity = dht.readHumidity();
    float current_temperature = dht.readTemperature();

    bool is_LIGHT_on = false;
    bool is_FAN_on = false;
    bool is_HEATER_on = false;
    bool isWatering = (digitalRead(PUMP_PIN) == HIGH); // Moved up for LCD display
    
    // --- LIGHT CONTROL ---
    int current_light_val;
    static uint32_t light_tmr;
    if (millis() - light_tmr >= LIGHT_check_period) {
        light_tmr = millis();
        current_light_val = analogRead(LIGHT_SENSOR_PIN);
        if (current_light_val < MIN_light_val && current_temperature <= MAX_plant_temperature) {
            is_LIGHT_on = true;
        } else {
            is_LIGHT_on = false;
        }
    }
    digitalWrite(LIGHT_PANEL_PIN, is_LIGHT_on);

    // --- SENSOR DATA UPDATE ---
    static uint32_t dht_tmr;
    if (millis() - dht_tmr >= DHT_check_period) {
        dht_tmr = millis(); 
        if (!isnan(current_temperature) && !isnan(current_humidity)) {
            update_Max_Min_temperature(current_temperature);
            update_Max_Min_humidity(current_humidity);
        }
    }
    
    // --- CLIMATE CONTROL ---
    static int32_t tmp_tmr;
    if (millis() - tmp_tmr >= tmp_period) {
        tmp_tmr = millis();
        if (current_temperature < MIN_plant_temperature) {
            is_FAN_on = true;
            is_HEATER_on = true;
        }
        else if (current_temperature > MAX_plant_temperature) {
            is_FAN_on = true;
        }
        else {
            is_FAN_on = false;
            is_HEATER_on = false;
        }
        digitalWrite(FAN_PIN, is_FAN_on);
        digitalWrite(HEATER_PIN, is_HEATER_on);
    }

    // --- LCD DISPLAY LOGIC (Matches the scheme) ---
    switch (OPTION) {
        case 1:
            // Line 0: 05:25   T: 22.5C° [fan_symb]
            lcd.setCursor(0, 0);
            printTwoDigits(now.hour());
            lcd.print(":");
            printTwoDigits(now.minute()); 
            
            lcd.setCursor(8, 0);
            lcd.print("T:");
            lcd.setCursor(11, 0);
            lcd.print(current_temperature, 1); // 1 decimal place to save space
            lcd.print("C");
            lcd.write((byte)0); // Degree symbol
            
            lcd.setCursor(15, 0);
            if (is_FAN_on) lcd.write((byte)1); else lcd.print(" ");

            // Line 1: Opt1  H: 45%     [water_symb]
            lcd.setCursor(0, 1);
            lcd.print("Opt");
            lcd.print(OPTION);
            
            lcd.setCursor(8, 1);
            lcd.print("H:");
            lcd.setCursor(11, 1);
            lcd.print(current_humidity);
            lcd.print("% ");
            
            lcd.setCursor(15, 1);
            if (isWatering) lcd.write((byte)2); else lcd.print(" ");
            break;

        case 2:
            // Line 0:       MAX t: 25.0°C
            lcd.setCursor(3, 0);
            lcd.print("MAX t: ");
            lcd.print(MAX_temperature, 1);
            lcd.print("C");
            lcd.write((byte)0);

            // Line 1: Opt2  MIN t: 18.0°C
            lcd.setCursor(0, 1);
            lcd.print("Opt");
            lcd.print(OPTION);
            lcd.setCursor(7, 1);
            lcd.print("MIN t: ");
            lcd.print(MIN_temperature, 1);
            lcd.print("C");
            lcd.write((byte)0);
            break;
            
        case 3:
            // Line 0:       MAX h: 60%
            lcd.setCursor(3, 0);
            lcd.print("MAX h: ");
            lcd.print(MAX_humidity);
            lcd.print("%");

            // Line 1: Opt3  MIN h: 40%
            lcd.setCursor(0, 1);
            lcd.print("Opt");
            lcd.print(OPTION);
            lcd.setCursor(7, 1);
            lcd.print("MIN h: ");
            lcd.print(MIN_humidity);
            lcd.print("%");
            break;
    }

    // --- WATERING LOGIC ---
    if (now.hour() == WATERING_HOUR && now.minute() == WATERING_MIN && !isWateredToday) {
        if (digitalRead(PUMP_PIN) == LOW) {
            digitalWrite(PUMP_PIN, HIGH); 
            PUMP_tmr = millis(); 
            buzz_pic(2, 25);
        }
        
        if (millis() - PUMP_tmr >= WATERING_DURATION) {
            digitalWrite(PUMP_PIN, LOW);
            isWateredToday = true; 
        }
    }

    if (now.minute() != WATERING_MIN) {
        isWateredToday = false;
    }

    // --- RGB LED STATE MACHINE ---
    if (isWatering) {
        RGB_watering();
    } 
    else if (is_HEATER_on) {
        RGB_heatering();
    } 
    else {
        analogWrite(RED_pin, 0);
        analogWrite(GREEN_pin, 0);
        analogWrite(BLUE_pin, 0);
    }

    // --- BUTTON HANDLING ---
    current_Mode_Button = debounce(last_Mode_Button, Mode_Button_Pin);
    current_Plus_Button = debounce(last_Plus_Button, Plus_Button_Pin);

    if (last_Plus_Button == LOW && current_Plus_Button == HIGH) {
        OPTION++;
        if (OPTION > MAX_OPTION) {
            OPTION = MIN_OPTION;
        }
    }

    last_Mode_Button = current_Mode_Button;
    last_Plus_Button = current_Plus_Button;
    last_Minus_Button = current_Minus_Button; 
}
