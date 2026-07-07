#include "JavaOptimizedFlags.h"

#include <algorithm>

namespace JavaOptimizedFlags {

namespace {

// Client-adapted Aikar G1 set. On Java 8 this replaces the default ParallelGC
// (whose stop-the-world collections are the classic 1.8.9 periodic stutter);
// on modern Java it tightens the already-default G1 for client frame pacing.
QStringList balancedG1(int javaMajor)
{
    QStringList args;
    // must come first: unlocks G1NewSizePercent/G1ReservePercent below
    args << "-XX:+UnlockExperimentalVMOptions";
    args << "-XX:+UseG1GC";
    // pause target ~2 frames @60fps instead of the 200ms server default
    args << "-XX:MaxGCPauseMillis=37";
    args << "-XX:G1NewSizePercent=20";
    // headroom against to-space exhaustion -> avoids fallback full GC
    args << "-XX:G1ReservePercent=20";
    // with 2-4G heaps the default region size makes chunk/packet arrays "humongous"
    args << "-XX:G1HeapRegionSize=8M";
    // start concurrent marking early so mixed collections stay cheap
    args << "-XX:InitiatingHeapOccupancyPercent=15";
    args << "-XX:G1MixedGCCountTarget=4";
    args << "-XX:G1MixedGCLiveThresholdPercent=90";
    args << "-XX:G1RSetUpdatingPauseTimePercent=5";
    // client garbage dies young: more eden, don't copy survivors around
    args << "-XX:SurvivorRatio=32";
    args << "-XX:MaxTenuringThreshold=1";
    if (javaMajor < 10) {
        // default since JDK 10; shortens remark pauses on 8/9
        args << "-XX:+ParallelRefProcEnabled";
    }
    // avoids rare multi-ms hiccups from hsperfdata mmap writes
    args << "-XX:+PerfDisableSharedMem";
    // some mods call System.gc(): a full stop-the-world hitch
    args << "-XX:+DisableExplicitGC";
    // commit heap pages at startup instead of page-faulting mid-game
    args << "-XX:+AlwaysPreTouch";
    return args;
}

QStringList lowLatencyZgc(int javaMajor)
{
    QStringList args;
    args << "-XX:+UseZGC";
    if (javaMajor <= 23) {
        // generational ZGC; default in 23, flag obsolete from 24 on
        args << "-XX:+ZGenerational";
    }
    args << "-XX:+AlwaysPreTouch";
    return args;
}

}  // namespace

QStringList generate(const Input& in)
{
    if (in.javaMajor <= 0)
        return {};

    // ZGC trades throughput and RAM for near-zero pauses; only worth it on a
    // modern JVM with heap and system memory to spare. Otherwise fall back.
    if (in.preset == "zgc" && in.javaMajor >= 21 && in.maxHeapMiB >= 6144 && in.totalRamMiB >= 16384)
        return lowLatencyZgc(in.javaMajor);

    return balancedG1(in.javaMajor);
}

HeapSizing recommendHeap(int javaMajor, uint64_t totalRamMiB, bool javaIs64bit)
{
    const qint64 totalRam = qint64(totalRamMiB);

    // 1.8.9-class games run best on 2-3G (bigger heaps just lengthen G1 cycles);
    // modern MC wants 4G as a baseline.
    qint64 max;
    if (javaMajor <= 8)
        max = std::clamp<qint64>(totalRam / 4, 2048, 3072);
    else
        max = 4096;

    max = std::min(max, totalRam / 2);
    if (!javaIs64bit)
        max = std::min<qint64>(max, 1536);
    max = std::max<qint64>(max, 1024);

    HeapSizing out;
    out.maxMiB = int(max);
    // with RAM to spare, Xms == Xmx removes heap-resize stutter entirely
    // (pairs with -XX:+AlwaysPreTouch from the flag set)
    out.minMiB = (totalRam >= 3 * max) ? int(max) : 512;
    return out;
}

}  // namespace JavaOptimizedFlags
