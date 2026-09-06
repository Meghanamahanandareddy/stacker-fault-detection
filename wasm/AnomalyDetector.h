#pragma once
#include <string>
#include <vector>

// This is the real detection logic, extracted from firmware/src/main.c's
// vAnomalyDetectTask, so the exact same thresholds and decision logic
// used in the FreeRTOS simulation also drive the browser dashboard --
// compiled once, used in both places, instead of reimplemented in JS.

enum class FaultKind {
    NONE,
    VIBRATION_HIGH,
    CURRENT_SPIKE,
    TEMPERATURE_HIGH
};

struct FaultLogEntry {
    double timestampS;
    FaultKind kind;
    double value;
};

class AnomalyDetector {
public:
    // Same threshold values as firmware/src/main.c -- keep these in sync
    // if you ever tune the FreeRTOS simulation's thresholds.
    static constexpr double VIBRATION_THRESHOLD_G = 2.5;
    static constexpr double CURRENT_THRESHOLD_A = 8.0;
    static constexpr double TEMPERATURE_THRESHOLD_C = 75.0;

    AnomalyDetector();

    // Checks all three sensors independently (no early exit), so
    // simultaneous faults are all detected and logged. Returns how many
    // faults this single reading triggered (0-3).
    int evaluate(double timestampS, double vibrationG, double currentA, double temperatureC);

    // All fault kind names detected on the most recent evaluate() call,
    // in vibration/current/temperature check order. Empty if none.
    std::vector<std::string> getAllFaultKindsThisTick() const;

    // The most recently logged single fault (highest-priority /
    // last-checked one from the most recent evaluate() call that found
    // at least one fault).
    std::string getLastFaultKindName() const;
    double getLastFaultValue() const;
    double getLastFaultTimestamp() const;

    int getFaultCount() const;

    static std::string faultKindName(FaultKind k);

private:
    std::vector<FaultLogEntry> faultLog_;
    std::vector<std::string> lastTickFaultKinds_;
};