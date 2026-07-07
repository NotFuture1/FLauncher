#pragma once

#include <QString>

// Windows-only game-process tuning. Everything here is best-effort:
// functions return false on failure and callers must log and continue —
// never block a launch on a tuning call.
namespace WindowsPerformance {

// priority: "abovenormal" | "high"
bool setProcessPriority(qint64 pid, const QString& priority);

}  // namespace WindowsPerformance
