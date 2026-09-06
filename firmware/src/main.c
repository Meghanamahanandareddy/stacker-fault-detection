#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "sensor_types.h"

// =================================================================
// FreeRTOS required hook functions
// =================================================================
void vApplicationMallocFailedHook(void) {
    printf("MALLOC FAILED - out of heap memory\n");
    fflush(stdout);
    exit(1);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    printf("STACK OVERFLOW in task: %s\n", pcTaskName);
    fflush(stdout);
    exit(1);
}

void vAssertCalled(const char *file, int line) {
    printf("ASSERT FAILED: %s:%d\n", file, line);
    fflush(stdout);
    exit(1);
}

// =================================================================
// Shared queues (the actual inter-task communication mechanism)
// =================================================================
static QueueHandle_t sensorQueue;
static QueueHandle_t alertQueue;

// =================================================================
// Fault-detection thresholds
// =================================================================
#define VIBRATION_THRESHOLD_G     2.5
#define CURRENT_THRESHOLD_A       8.0
#define TEMPERATURE_THRESHOLD_C   75.0

static FILE *csvLog = NULL;

// =================================================================
// TASK 1: Sensor polling (Priority 3)
// =================================================================
void vSensorPollTask(void *pv) {
    (void) pv;
    double simTimeS = 0.0;
    const double dt = 0.2;

    srand(42);

    for (;;) {
        double vibration = 0.6 + ((double) rand() / RAND_MAX) * 0.4;
        double current   = 3.5 + ((double) rand() / RAND_MAX) * 1.0;
        double temperature = 45.0 + ((double) rand() / RAND_MAX) * 8.0;

        if (simTimeS >= 10.0 && simTimeS < 16.0) {
            double faultProgress = (simTimeS - 10.0) / 6.0;
            vibration += faultProgress * 2.5;
            temperature += faultProgress * 25.0;
            if (simTimeS >= 13.0 && simTimeS < 13.6) {
                current += 6.0;
            }
        }

        SensorReading reading = { simTimeS, vibration, current, temperature };
        xQueueSend(sensorQueue, &reading, portMAX_DELAY);

        simTimeS += dt;
        vTaskDelay(pdMS_TO_TICKS((int)(dt * 1000)));
    }
}

// =================================================================
// TASK 2: Anomaly detection (Priority 2)
// =================================================================
void vAnomalyDetectTask(void *pv) {
    (void) pv;
    SensorReading reading;

    for (;;) {
        if (xQueueReceive(sensorQueue, &reading, portMAX_DELAY) == pdPASS) {
            if (csvLog) {
                fprintf(csvLog, "%.2f,%.3f,%.3f,%.3f\n",
                        reading.timestampS, reading.vibrationG,
                        reading.currentA, reading.temperatureC);
                fflush(csvLog);
            }

            FaultEvent fault = { reading.timestampS, FAULT_NONE, 0.0 };
            int faultDetected = 0;

            if (reading.vibrationG > VIBRATION_THRESHOLD_G) {
                fault.type = FAULT_VIBRATION_HIGH;
                fault.value = reading.vibrationG;
                faultDetected = 1;
            } else if (reading.currentA > CURRENT_THRESHOLD_A) {
                fault.type = FAULT_CURRENT_SPIKE;
                fault.value = reading.currentA;
                faultDetected = 1;
            } else if (reading.temperatureC > TEMPERATURE_THRESHOLD_C) {
                fault.type = FAULT_TEMPERATURE_HIGH;
                fault.value = reading.temperatureC;
                faultDetected = 1;
            }

            if (faultDetected) {
                xQueueSend(alertQueue, &fault, 0);
            }
        }
    }
}

// =================================================================
// TASK 3: Fault flagging (Priority 4 -- HIGHEST)
// =================================================================
const char* faultTypeName(FaultType t) {
    switch (t) {
        case FAULT_VIBRATION_HIGH: return "VIBRATION_HIGH";
        case FAULT_CURRENT_SPIKE: return "CURRENT_SPIKE";
        case FAULT_TEMPERATURE_HIGH: return "TEMPERATURE_HIGH";
        default: return "NONE";
    }
}

void vFaultFlagTask(void *pv) {
    (void) pv;
    FaultEvent fault;

    for (;;) {
        if (xQueueReceive(alertQueue, &fault, portMAX_DELAY) == pdPASS) {
            printf("[%.2fs] *** FAULT DETECTED: %s (value=%.2f) ***\n",
                   fault.timestampS, faultTypeName(fault.type), fault.value);
            fflush(stdout);
        }
    }
}

// =================================================================
// TASK 4: Comms (Priority 1 -- LOWEST)
// =================================================================
void vCommsTask(void *pv) {
    (void) pv;
    int count = 0;
    for (;;) {
        printf("[comms] telemetry heartbeat #%d sent to supervisor\n", count++);
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int main(void) {
    printf("Starting Stacker Crane Predictive Maintenance simulation...\n");

    csvLog = fopen("data/telemetry_log.csv", "w");
    if (csvLog) {
        fprintf(csvLog, "time_s,vibration_g,current_a,temperature_c\n");
    } else {
        printf("Warning: could not open data/telemetry_log.csv for writing\n");
    }

    sensorQueue = xQueueCreate(20, sizeof(SensorReading));
    alertQueue  = xQueueCreate(10, sizeof(FaultEvent));

    xTaskCreate(vSensorPollTask,    "SensorPoll", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    xTaskCreate(vAnomalyDetectTask, "Anomaly",    configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vFaultFlagTask,     "FaultFlag",  configMINIMAL_STACK_SIZE, NULL, 4, NULL);
    xTaskCreate(vCommsTask,         "Comms",      configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();

    printf("Scheduler failed to start.\n");
    return 1;
}