# Stacker Crane Predictive Maintenance & Real-Time Fault Detection

A real-time embedded monitoring system for industrial machinery (motors, actuators, lift mechanisms), built around **FreeRTOS** task scheduling with genuine threshold-based anomaly detection — designed as the real-time-systems counterpart to [agv-motion-controller](https://github.com/VISHNU2305/agv-motion-controller).

## What this demonstrates

A simulated stacker-crane-style motor rig monitors vibration, current, and temperature concurrently using four independent FreeRTOS tasks, each with a deliberately chosen priority:

| Task | Priority | Why |
|---|---|---|
| Fault Flagging | 4 (highest) | A missed fault is unrecoverable |
| Sensor Polling | 3 | Feeds everything downstream |
| Anomaly Detection | 2 | Consumes sensor data, evaluates thresholds |
| Comms/Telemetry | 1 (lowest) | A missed heartbeat is recoverable — safe to deprioritize |

This priority ordering is a deliberate, explainable design decision — not an arbitrary choice — and is the core "real-time systems" argument for this project.

## Tech Stack

| Layer | Technology |
|---|---|
| Real-time task scheduling | FreeRTOS (POSIX simulator port) |
| Core simulation logic | C (C17), compiled with gcc |
| Fault detection (live browser demo) | C++17, compiled to WebAssembly via Emscripten |
| 3D/UI rendering | HTML, CSS, JavaScript |
| Data visualization | Python, matplotlib, pandas |
| Development environment | WSL2 (Ubuntu) on Windows |
| Version control | Git / GitHub |

## Project structure

stacker-fault-detection/
├── firmware/
│ ├── config/FreeRTOSConfig.h # FreeRTOS configuration
│ └── src/
│ ├── sensor_types.h # shared data structures (queues)
│ └── main.c # 4-task architecture, runs the simulation
├── wasm/ # C++ fault-detection logic, compiled to WebAssembly
│ ├── AnomalyDetector.h/.cpp # same threshold logic as firmware/src/main.c
│ └── bindings.cpp # Embind bridge exposing it to JavaScript
├── dashboard/ # interactive live browser dashboard
│ └── index.html # driven directly by compiled anomaly.wasm
├── ml/
│ └── plot_telemetry.py # visualizes a recorded run's sensor data
├── data/ # telemetry_log.csv output lands here
└── docs/ # diagrams

Note: firmware/FreeRTOS-Kernel/ (the cloned FreeRTOS source) is not committed
— see "Setup" below to fetch it.


## Setup

Requires WSL2 (Ubuntu) if on Windows — FreeRTOS's POSIX simulator needs a Linux environment.

```bash
# 1. Clone this repo
git clone https://github.com/VISHNU2305/stacker-fault-detection.git
cd stacker-fault-detection

# 2. Fetch FreeRTOS's kernel source (not committed to this repo)
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git firmware/FreeRTOS-Kernel
```

## Build and run the FreeRTOS simulation

```bash
gcc -DprojCOVERAGE_TEST=0 \
  -I firmware/config \
  -I firmware/FreeRTOS-Kernel/include \
  -I firmware/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix \
  -I firmware/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/utils \
  firmware/src/main.c \
  firmware/FreeRTOS-Kernel/tasks.c \
  firmware/FreeRTOS-Kernel/queue.c \
  firmware/FreeRTOS-Kernel/list.c \
  firmware/FreeRTOS-Kernel/timers.c \
  firmware/FreeRTOS-Kernel/event_groups.c \
  firmware/FreeRTOS-Kernel/portable/MemMang/heap_3.c \
  firmware/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/port.c \
  firmware/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c \
  -lpthread -lm -o firmware/rtos_sim

./firmware/rtos_sim
```

A fault is deliberately injected between t=10s and t=16s (simulating a developing bearing fault), so you should see normal telemetry heartbeats, then real-time fault detections as vibration/current/temperature exceed their thresholds, then a return to normal.

## Visualize a recorded run

```bash
python3 -m venv venv
source venv/bin/activate
pip install matplotlib pandas
cd ml
python3 plot_telemetry.py
```

Produces `ml/telemetry_plot.png` — three stacked charts (vibration, current, temperature) with the fault window highlighted.

## Live interactive dashboard

The dashboard is driven directly by the same threshold-detection logic as `firmware/src/main.c`, compiled to WebAssembly — not a separate JavaScript reimplementation.

```bash
cd dashboard
python3 -m http.server 8000
```
Open `http://localhost:8000`. Drag the sliders or use the "Inject Fault" buttons to push a reading past its threshold and watch the real compiled C++ logic detect it live — including detecting multiple simultaneous faults.

**Recompile after changing `wasm/AnomalyDetector.cpp`:**
```bash
cd wasm
em++ -std=c++17 AnomalyDetector.cpp bindings.cpp --bind -s MODULARIZE=1 -s EXPORT_NAME="createAnomalyModule" -s ENVIRONMENT=web -O2 -o ../dashboard/anomaly.js
```

## Honest scope notes

- **Detection approach:** threshold-based, not full ML-on-microcontroller — this matches what most real industrial edge systems actually run in production. A TinyML upgrade path (training an anomaly model offline, deploying via Edge Impulse) is a natural next step.
- **Security:** the project plan called for Secure Boot / embedded cryptography. That requires vendor-specific hardware provisioning tools not available in a pure-software simulation, so it's intentionally out of scope here rather than faked — a real next step with hardware access.
- **Simulated sensors:** vibration/current/temperature are synthetic (with a deliberately injected fault window for demonstration), not from physical hardware — this is a software-in-the-loop simulation, same philosophy as [agv-motion-controller](https://github.com/VISHNU2305/agv-motion-controller).

## Related project

[agv-motion-controller](https://github.com/VISHNU2305/agv-motion-controller) — the motion-control/navigation half of this two-project pair, covering C++ state machines, UML modeling, and obstacle rerouting.

Authors:- Meghana Reddy , Pendyala Vishnu sai

