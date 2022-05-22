#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>

#define RED_PIN 9
#define BLUE_PIN 11
#define goodTemp 20
#define offetVoltage 500
#define temp A0

typedef struct readRawData
{
  int pin;
  int data;
} rawData;

QueueHandle_t structQueue;

// Function Declaration 
void getTempValue(void *pvParameters);    // Task to get the tempeature value from the thermistor 
void ledColourChange(void *pvParameters); // Task to toggle the Led 

void setup()
{
  Serial.begin(9600);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(temp, INPUT);

// Setup the tasks
  structQueue = xQueueCreate(10,
                             sizeof(rawData));

  if (structQueue != NULL)
  {

    xTaskCreate(getTempValue,
                "tempADC",
                128,
                NULL,
                1,
                NULL);

    xTaskCreate(ledColourChange,
                "ledColour",
                128,
                NULL,
                1,
                NULL);
  }
}
void loop() {}

// Task 1 : Get the Raw temperature value from the thermistor 
void getTempValue(void *pvParameters)
{
  (void)pvParameters;

  for (;;)
  {
    rawData tempSensor;
    tempSensor.pin = temp;
    tempSensor.data = analogRead(temp); // read the raw data from the sensor
    xQueueSend(structQueue, &tempSensor, portMAX_DELAY); // update the structure with this data 
  }
  taskYIELD();
}

// Task 2 : Change the LED that glows based on the tempertaure 
void ledColourChange(void *pvParameters)
{
  (void)pvParameters;
  for (;;)
  {
    int tempVoltage = 0;
    int tempValue = 0;

    rawData readValue;

    if (xQueueReceive(structQueue, &readValue, portMAX_DELAY) == pdPASS)
    {
      tempVoltage = readValue.data * 3.22;
      tempValue = offetVoltage - tempVoltage;
      
      Serial.println(tempValue);

      if (tempValue > goodTemp)
      {
        /* If the temperature exeeds the required constaant temperature turn the led to red */
        digitalWrite(RED_PIN, HIGH);
        digitalWrite(BLUE_PIN, LOW);

        delay(1000);
      }
      else
      {
        // The temperature is under the required minimum value keep the blue led on
        digitalWrite(BLUE_PIN, HIGH);
        digitalWrite(RED_PIN, LOW);

        delay(1000);
      }
    }
  }
  taskYIELD();
}

