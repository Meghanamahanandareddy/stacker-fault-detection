#ifndef SENSOR_TYPES_H
#define SENSOR_TYPES_H

// A single sensor sample, pushed onto sensorQueue by vSensorPollTask
// and consumed by vAnomalyDetectTask.
typedef struct {
    double timestampS;
    double vibrationG;      // simulated accelerometer reading, in g's
    double currentA;        // simulated motor current, in amps
    double temperatureC;    // simulated bearing/motor temperature, in Celsius
} SensorReading;

// A detected fault, pushed onto alertQueue by vAnomalyDetectTask
// and consumed by vFaultFlagTask.
typedef enum {
    FAULT_NONE = 0,
    FAULT_VIBRATION_HIGH,
    FAULT_CURRENT_SPIKE,
    FAULT_TEMPERATURE_HIGH
} FaultType;

typedef struct {
    double timestampS;
    FaultType type;
    double value;       // the offending reading that triggered this fault
} FaultEvent;

#endif // SENSOR_TYPES_H