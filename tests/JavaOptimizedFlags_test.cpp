#include <QTest>

#include <java/JavaOptimizedFlags.h>

using namespace JavaOptimizedFlags;

class JavaOptimizedFlagsTest : public QObject {
    Q_OBJECT

    static Input makeInput(int major, uint64_t ramMiB = 16384, int heapMiB = 4096, const QString& preset = "balanced")
    {
        Input in;
        in.javaMajor = major;
        in.totalRamMiB = ramMiB;
        in.maxHeapMiB = heapMiB;
        in.preset = preset;
        return in;
    }

   private slots:
    void test_java8Balanced()
    {
        const auto args = generate(makeInput(8));
        QVERIFY(!args.isEmpty());
        // unlock flag must precede the experimental flags it unlocks
        QCOMPARE(args.first(), QString("-XX:+UnlockExperimentalVMOptions"));
        QVERIFY(args.contains("-XX:+UseG1GC"));
        QVERIFY(args.contains("-XX:MaxGCPauseMillis=37"));
        QVERIFY(args.contains("-XX:G1NewSizePercent=20"));
        QVERIFY(args.contains("-XX:+ParallelRefProcEnabled"));
        QVERIFY(args.contains("-XX:+DisableExplicitGC"));
        QVERIFY(args.contains("-XX:+AlwaysPreTouch"));
    }

    void test_java17Balanced()
    {
        const auto args = generate(makeInput(17));
        QCOMPARE(args.first(), QString("-XX:+UnlockExperimentalVMOptions"));
        QVERIFY(args.contains("-XX:+UseG1GC"));
        // default since JDK 10, must not be re-emitted
        QVERIFY(!args.contains("-XX:+ParallelRefProcEnabled"));
    }

    void test_zgcPreset()
    {
        const auto args = generate(makeInput(21, 32768, 8192, "zgc"));
        QVERIFY(args.contains("-XX:+UseZGC"));
        QVERIFY(args.contains("-XX:+ZGenerational"));
        QVERIFY(args.contains("-XX:+AlwaysPreTouch"));
        QVERIFY(!args.contains("-XX:+UseG1GC"));
    }

    void test_zgcGenerationalObsoleteOn24()
    {
        const auto args = generate(makeInput(24, 32768, 8192, "zgc"));
        QVERIFY(args.contains("-XX:+UseZGC"));
        QVERIFY(!args.contains("-XX:+ZGenerational"));
    }

    void test_zgcFallsBackToBalanced_data()
    {
        QTest::addColumn<int>("major");
        QTest::addColumn<quint64>("ramMiB");
        QTest::addColumn<int>("heapMiB");

        QTest::newRow("java too old") << 17 << quint64(32768) << 8192;
        QTest::newRow("never on java 8") << 8 << quint64(32768) << 8192;
        QTest::newRow("not enough ram") << 21 << quint64(8192) << 8192;
        QTest::newRow("heap too small") << 21 << quint64(32768) << 4096;
    }
    void test_zgcFallsBackToBalanced()
    {
        QFETCH(int, major);
        QFETCH(quint64, ramMiB);
        QFETCH(int, heapMiB);

        const auto args = generate(makeInput(major, ramMiB, heapMiB, "zgc"));
        QVERIFY(!args.contains("-XX:+UseZGC"));
        QVERIFY(args.contains("-XX:+UseG1GC"));
    }

    void test_unknownJavaGeneratesNothing()
    {
        QVERIFY(generate(makeInput(0)).isEmpty());
        QVERIFY(generate(makeInput(-1)).isEmpty());
    }

    void test_neverEmitsHeapFlags()
    {
        for (int major : { 0, 8, 17, 21, 24 }) {
            for (const QString& preset : { QString("balanced"), QString("zgc") }) {
                for (const auto& arg : generate(makeInput(major, 32768, 8192, preset))) {
                    QVERIFY2(!arg.startsWith("-Xms"), qPrintable(arg));
                    QVERIFY2(!arg.startsWith("-Xmx"), qPrintable(arg));
                    QVERIFY2(!arg.contains("PermSize"), qPrintable(arg));
                }
            }
        }
    }

    void test_recommendHeap_data()
    {
        QTest::addColumn<int>("major");
        QTest::addColumn<quint64>("ramMiB");
        QTest::addColumn<bool>("is64bit");
        QTest::addColumn<int>("expectedMax");
        QTest::addColumn<int>("expectedMin");

        // java 8 band: ram/4 clamped to 2048-3072, capped at ram/2
        QTest::newRow("java8 4G ram") << 8 << quint64(4096) << true << 2048 << 512;
        QTest::newRow("java8 8G ram") << 8 << quint64(8192) << true << 2048 << 2048;   // 8192 >= 3*2048 -> Xms==Xmx
        QTest::newRow("java8 16G ram") << 8 << quint64(16384) << true << 3072 << 3072;
        // modern baseline 4096, Xms==Xmx only when ram >= 3*max
        QTest::newRow("java17 8G ram") << 17 << quint64(8192) << true << 4096 << 512;
        QTest::newRow("java17 32G ram") << 17 << quint64(32768) << true << 4096 << 4096;
        // half-of-ram cap and the 1024 floor
        QTest::newRow("java17 4G ram") << 17 << quint64(4096) << true << 2048 << 512;
        QTest::newRow("tiny ram floor") << 17 << quint64(1024) << true << 1024 << 512;
        // 32-bit address-space safety
        QTest::newRow("32-bit java") << 8 << quint64(16384) << false << 1536 << 1536;
    }
    void test_recommendHeap()
    {
        QFETCH(int, major);
        QFETCH(quint64, ramMiB);
        QFETCH(bool, is64bit);
        QFETCH(int, expectedMax);
        QFETCH(int, expectedMin);

        const auto rec = recommendHeap(major, ramMiB, is64bit);
        QCOMPARE(rec.maxMiB, expectedMax);
        QCOMPARE(rec.minMiB, expectedMin);
    }
};

QTEST_GUILESS_MAIN(JavaOptimizedFlagsTest)

#include "JavaOptimizedFlags_test.moc"
