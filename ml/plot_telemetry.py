import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("../data/telemetry_log.csv")

fig, axes = plt.subplots(3, 1, figsize=(11, 9), sharex=True)

VIBRATION_THRESHOLD = 2.5
CURRENT_THRESHOLD = 8.0
TEMPERATURE_THRESHOLD = 75.0

axes[0].plot(df["time_s"], df["vibration_g"], color="#2266cc", linewidth=1)
axes[0].axhline(y=VIBRATION_THRESHOLD, color="red", linestyle="--", linewidth=1, label=f"threshold ({VIBRATION_THRESHOLD}g)")
axes[0].set_ylabel("Vibration (g)")
axes[0].set_title("Simulated Stacker Crane Motor Telemetry — Predictive Maintenance Run")
axes[0].legend(fontsize=8, loc="upper right")
axes[0].grid(True, alpha=0.3)

axes[1].plot(df["time_s"], df["current_a"], color="#cc6622", linewidth=1)
axes[1].axhline(y=CURRENT_THRESHOLD, color="red", linestyle="--", linewidth=1, label=f"threshold ({CURRENT_THRESHOLD}A)")
axes[1].set_ylabel("Current (A)")
axes[1].legend(fontsize=8, loc="upper right")
axes[1].grid(True, alpha=0.3)

axes[2].plot(df["time_s"], df["temperature_c"], color="#22aa66", linewidth=1)
axes[2].axhline(y=TEMPERATURE_THRESHOLD, color="red", linestyle="--", linewidth=1, label=f"threshold ({TEMPERATURE_THRESHOLD}C)")
axes[2].set_ylabel("Temperature (C)")
axes[2].set_xlabel("Time (s)")
axes[2].legend(fontsize=8, loc="upper right")
axes[2].grid(True, alpha=0.3)

for ax in axes:
    ax.axvspan(10.0, 16.0, color="red", alpha=0.08)

plt.tight_layout()
plt.savefig("telemetry_plot.png", dpi=150)
print("Saved chart to ml/telemetry_plot.png")
plt.show()


