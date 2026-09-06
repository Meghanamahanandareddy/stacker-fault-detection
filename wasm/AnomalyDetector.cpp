#include "AnomalyDetector.h"

AnomalyDetector::AnomalyDetector() {}

std::string AnomalyDetector::faultKindName(FaultKind k) {
    switch (k) {
        case FaultKind::VIBRATION_HIGH: return "VIBRATION_HIGH";
        case FaultKind::CURRENT_SPIKE: return "CURRENT_SPIKE";
        case FaultKind::TEMPERATURE_HIGH: return "TEMPERATURE_HIGH";
        default: return "NONE";
    }
}

int AnomalyDetector::evaluate(double timestampS, double vibrationG, double currentA, double temperatureC) {
    lastTickFaultKinds_.clear();
    int faultCount = 0;

    // Each sensor is checked independently -- no early exit -- so
    // multiple simultaneous faults are all detected and logged, even
    // though the dashboard may choose to headline only the most recent one.
    if (vibrationG > VIBRATION_THRESHOLD_G) {
        faultLog_.push_back({timestampS, FaultKind::VIBRATION_HIGH, vibrationG});
        lastTickFaultKinds_.push_back(faultKindName(FaultKind::VIBRATION_HIGH));
        faultCount++;
    }
    if (currentA > CURRENT_THRESHOLD_A) {
        faultLog_.push_back({timestampS, FaultKind::CURRENT_SPIKE, currentA});
        lastTickFaultKinds_.push_back(faultKindName(FaultKind::CURRENT_SPIKE));
        faultCount++;
    }
    if (temperatureC > TEMPERATURE_THRESHOLD_C) {
        faultLog_.push_back({timestampS, FaultKind::TEMPERATURE_HIGH, temperatureC});
        lastTickFaultKinds_.push_back(faultKindName(FaultKind::TEMPERATURE_HIGH));
        faultCount++;
    }

    return faultCount;
}

std::vector<std::string> AnomalyDetector::getAllFaultKindsThisTick() const {
    return lastTickFaultKinds_;
}

std::string AnomalyDetector::getLastFaultKindName() const {
    if (faultLog_.empty()) return "NONE";
    return faultKindName(faultLog_.back().kind);
}

double AnomalyDetector::getLastFaultValue() const {
    if (faultLog_.empty()) return 0.0;
    return faultLog_.back().value;
}

double AnomalyDetector::getLastFaultTimestamp() const {
    if (faultLog_.empty()) return -1.0;
    return faultLog_.back().timestampS;
}

int AnomalyDetector::getFaultCount() const {
    return static_cast<int>(faultLog_.size());
}