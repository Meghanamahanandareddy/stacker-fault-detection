#include <emscripten/bind.h>
#include <vector>
#include <string>
#include "AnomalyDetector.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(anomaly_module) {
    register_vector<std::string>("VectorString");

    enum_<FaultKind>("FaultKind")
        .value("NONE", FaultKind::NONE)
        .value("VIBRATION_HIGH", FaultKind::VIBRATION_HIGH)
        .value("CURRENT_SPIKE", FaultKind::CURRENT_SPIKE)
        .value("TEMPERATURE_HIGH", FaultKind::TEMPERATURE_HIGH)
        ;

    class_<AnomalyDetector>("AnomalyDetector")
        .constructor<>()
        .function("evaluate", &AnomalyDetector::evaluate)
        .function("getAllFaultKindsThisTick", &AnomalyDetector::getAllFaultKindsThisTick)
        .function("getLastFaultKindName", &AnomalyDetector::getLastFaultKindName)
        .function("getLastFaultValue", &AnomalyDetector::getLastFaultValue)
        .function("getLastFaultTimestamp", &AnomalyDetector::getLastFaultTimestamp)
        .function("getFaultCount", &AnomalyDetector::getFaultCount)
        ;
}