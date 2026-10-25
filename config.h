#pragma once

// DHT
#define DHT_PIN A0

// Lighting system
#define LIGHT_PANEL_PIN 12
#define LIGHT_SENSOR_PIN 10
#define LIGHT_check_period 200
#define MIN_light_val 500 // you should correct this data by yourself
#define MAX_light_val 1000 // and this too

// cooling system
#define HEATER_PIN 5
#define FAN_PIN 4

#define TMP_SENSOR_PIN 0 
#define DHT_check_period 2000

#define MAX_OPTION 5
#define MIN_OPTION 1

// temperature
#define tmp_period (1000 * 3) // added parentheses for safety
#define MIN_plant_temperature 15
#define MAX_plant_temperature 25

// RGB led
#define RED_pin A1
#define GREEN_pin A2
#define BLUE_pin A3

// RGB Effects
#define light_flash_period 100
#define MAX_light_WATERING_options 14
#define MAX_light_HEATERING_options 7

// buttons
#define Plus_Button_Pin 7
#define Minus_Button_Pin 8
#define Mode_Button_Pin 9

// watering settings
#define PUMP_PIN 6
#define WATERING_HOUR 14    // ]
#define WATERING_MIN 0      // ] -> it means that watering will on in 14:00
#define WATERING_DURATION 5000 

// buzzer
#define BUZZER_pin 3
