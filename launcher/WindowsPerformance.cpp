#include "WindowsPerformance.h"

#include <QDir>

#include <string>

#include <windows.h>

namespace {

// the registry VALUE NAME is the full exe path (backslashes and all), which is
// why this uses the raw Win32 API — QSettings treats path separators in value
// names as subkey separators and cannot address these entries
const wchar_t* kGpuPreferencesKey = L"Software\\Microsoft\\DirectX\\UserGpuPreferences";
const std::wstring kHighPerformanceGpu = L"GpuPreference=2;";

std::wstring toValueName(const QString& exeAbsolutePath)
{
    return QDir::toNativeSeparators(exeAbsolutePath).toStdWString();
}

}  // namespace

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

bool applyGpuPreference(const QString& exeAbsolutePath)
{
    const std::wstring name = toValueName(exeAbsolutePath);
    return RegSetKeyValueW(HKEY_CURRENT_USER, kGpuPreferencesKey, name.c_str(), REG_SZ, kHighPerformanceGpu.c_str(),
                           DWORD((kHighPerformanceGpu.size() + 1) * sizeof(wchar_t))) == ERROR_SUCCESS;
}

bool clearGpuPreferenceIfOurs(const QString& exeAbsolutePath)
{
    const std::wstring name = toValueName(exeAbsolutePath);
    wchar_t data[64];
    DWORD size = sizeof(data);
    if (RegGetValueW(HKEY_CURRENT_USER, kGpuPreferencesKey, name.c_str(), RRF_RT_REG_SZ, nullptr, data, &size) != ERROR_SUCCESS)
        return true;  // absent, or too big to be ours: nothing to clear
    if (std::wstring(data) != kHighPerformanceGpu)
        return true;  // set by the user, leave it alone
    return RegDeleteKeyValueW(HKEY_CURRENT_USER, kGpuPreferencesKey, name.c_str()) == ERROR_SUCCESS;
}

}  // namespace WindowsPerformance
