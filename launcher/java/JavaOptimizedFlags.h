#pragma once

#include <QString>
#include <QStringList>
#include <cstdint>

namespace JavaOptimizedFlags {

struct Input {
    int javaMajor = 0;         // JavaVersion::major(); 0 = unknown, generates nothing
    int javaSecurity = 0;      // JavaVersion::security(); the "update" number on Java 8
    uint64_t totalRamMiB = 0;  // HardwareInfo::totalRamMiB()
    int maxHeapMiB = 0;        // the resolved -Xmx that will actually be used
    QString preset;            // "balanced" (default) | "zgc"
};

// GC tuning flags only — never emits -Xms/-Xmx/-XX:PermSize.
// Emitted before the user's custom JvmArgs, so any flag the user sets
// explicitly wins (HotSpot resolves duplicate -XX flags last-one-wins).
QStringList generate(const Input& in);

struct HeapSizing {
    int minMiB = 512;
    int maxMiB = 0;
};

// Heap recommendation from installed RAM and the game's Java generation.
// Used when SmartHeapSizing is on and the instance has no memory override.
HeapSizing recommendHeap(int javaMajor, uint64_t totalRamMiB, bool javaIs64bit);

}  // namespace JavaOptimizedFlags
