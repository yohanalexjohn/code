#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>

#define RED_PIN 9
#define BLUE_PIN 11
#define goodTemp 20
#define offetVoltage 500
#define temp A0

void getTempValue(void *pvParameters);
void ledColourChange(void *pvParameters);

typedef struct readRawData
{
  int pin;
  int data;
} rawData;

QueueHandle_t structQueue;

void setup()
{
  Serial.begin(9600);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(temp, INPUT);

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

void getTempValue(void *pvParameters)
{
  (void)pvParameters;

  for (;;)
  {
    rawData tempSensor;
    tempSensor.pin = temp;
    tempSensor.data = analogRead(temp);
    xQueueSend(structQueue, &tempSensor, portMAX_DELAY);
  }
  taskYIELD();
}

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
      tempValue = -((tempVoltage - offetVoltage));
      Serial.println(tempValue);
      if (tempValue > goodTemp)
      {
        digitalWrite(RED_PIN, HIGH);
        digitalWrite(BLUE_PIN, LOW);

        delay(1000);
      }
      else
      {
        digitalWrite(BLUE_PIN, HIGH);
        digitalWrite(RED_PIN, LOW);
        delay(1000);
      }
    }
  }
  taskYIELD();
}

