#include "WindowsPerformance.h"

#include <windows.h>

namespace WindowsPerformance {

bool setProcessPriority(qint64 pid, const QString& priority)
{
    DWORD priorityClass;
    if (priority == "high")
        priorityClass = HIGH_PRIORITY_CLASS;
    else if (priority == "abovenormal")
        priorityClass = ABOVE_NORMAL_PRIORITY_CLASS;
    else
        return false;

    HANDLE handle = OpenProcess(PROCESS_SET_INFORMATION, FALSE, DWORD(pid));
    if (!handle)
        return false;
    const bool ok = SetPriorityClass(handle, priorityClass) != 0;
    CloseHandle(handle);
    return ok;
}

}  // namespace WindowsPerformance
