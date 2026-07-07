#pragma once

#include <QString>

// Windows-only game-process tuning. Everything here is best-effort:
// functions return false on failure and callers must log and continue —
// never block a launch on a tuning call.
namespace WindowsPerformance {

// priority: "abovenormal" | "high"
bool setProcessPriority(qint64 pid, const QString& priority);

// Registers exeAbsolutePath under HKCU DirectX\UserGpuPreferences so Windows
// runs it on the high-performance GPU. Must be called before the process is
// created; a no-op on single-GPU systems.
bool applyGpuPreference(const QString& exeAbsolutePath);

// Removes the entry written by applyGpuPreference, but only when its value is
// exactly ours — a preference the user set through Windows Settings is kept.
bool clearGpuPreferenceIfOurs(const QString& exeAbsolutePath);

}  // namespace WindowsPerformance
