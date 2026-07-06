// SPDX-License-Identifier: GPL-3.0-only
/*
 *  FLauncher - Minecraft Launcher
 *  Copyright (C) 2026 FLauncher Contributors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 */

#include <QTemporaryDir>
#include <QTest>

#include <minecraft/auth/AccountImport.h>

namespace {
const QString kPrismClientId = "c36a9fb6-4f2a-41ff-90bd-ae7cc92031eb";
const QString kMultiMCClientId = "499546d9-bbfe-4b9b-a086-eb3d75afb78f";
const QString kEssentialClientId = "e39cc675-eb52-4475-b5f8-82aaae14eeba";

QString writeTemp(QTemporaryDir& dir, const QString& name, const QByteArray& contents)
{
    QString path = dir.filePath(name);
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(contents);
    file.close();
    return path;
}

// Find a parsed candidate by username.
AccountImport::Candidate byName(const QList<AccountImport::Candidate>& list, const QString& name)
{
    for (const auto& c : list)
        if (c.username == name)
            return c;
    AccountImport::Candidate none;
    return none;
}
}  // namespace

class AccountImportTest : public QObject {
    Q_OBJECT
   private slots:

    void test_prism()
    {
        QTemporaryDir dir;
        QByteArray json = R"({
            "formatVersion": 3,
            "accounts": [
                { "type": "MSA", "msa-client-id": "c36a9fb6-4f2a-41ff-90bd-ae7cc92031eb",
                  "msa": { "refresh_token": "RT", "token": "AT" },
                  "profile": { "id": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "name": "PrismMSA" } },
                { "type": "Offline",
                  "profile": { "id": "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb", "name": "PrismOffline" } }
            ]
        })";
        QString path = writeTemp(dir, "accounts.json", json);

        QString source, error;
        auto candidates = AccountImport::parseFile(path, source, error);

        QCOMPARE(error, QString());
        QCOMPARE(source, QString("Prism Launcher"));
        QCOMPARE(candidates.size(), 2);

        auto msa = byName(candidates, "PrismMSA");
        QVERIFY(msa.account != nullptr);
        QCOMPARE(msa.typeName, QString("Microsoft"));
        // Prism accounts keep their own client id.
        QCOMPARE(msa.account->saveToJson().value("msa-client-id").toString(), kPrismClientId);

        auto offline = byName(candidates, "PrismOffline");
        QVERIFY(offline.account != nullptr);
        QCOMPARE(offline.typeName, QString("Offline"));
    }

    void test_multimc()
    {
        QTemporaryDir dir;
        // MultiMC's v3 format has no "msa-client-id"; import must inject MultiMC's.
        QByteArray json = R"({
            "formatVersion": 3,
            "accounts": [
                { "type": "MSA",
                  "msa": { "refresh_token": "RT", "token": "AT" },
                  "ygg": { "token": "MC" },
                  "profile": { "id": "cccccccccccccccccccccccccccccccc", "name": "MMCPlayer" } }
            ]
        })";
        QString path = writeTemp(dir, "accounts.json", json);

        QString source, error;
        auto candidates = AccountImport::parseFile(path, source, error);

        QCOMPARE(error, QString());
        QCOMPARE(source, QString("MultiMC"));
        QCOMPARE(candidates.size(), 1);
        QCOMPARE(candidates[0].username, QString("MMCPlayer"));
        QCOMPARE(candidates[0].account->saveToJson().value("msa-client-id").toString(), kMultiMCClientId);
    }

    void test_essential()
    {
        QTemporaryDir dir;
        QByteArray json = R"({
            "accounts": [
                {
                    "uuid": "069a79f4-44e9-4726-a5be-fca90e38aaf5",
                    "name": "EssPlayer",
                    "accessToken": "MC_ACCESS_TOKEN",
                    "auth": {
                        "accessToken": { "value": "MSA_ACCESS" },
                        "refreshToken": { "value": "MSA_REFRESH", "expires": { "seconds": 1700086400, "nanos": 0 } },
                        "profile": { "id": "069a79f4-44e9-4726-a5be-fca90e38aaf5", "name": "EssPlayer" }
                    }
                }
            ]
        })";
        QString path = writeTemp(dir, "microsoft_accounts.json", json);

        QString source, error;
        auto candidates = AccountImport::parseFile(path, source, error);

        QCOMPARE(error, QString());
        QCOMPARE(source, QString("Essential"));
        QCOMPARE(candidates.size(), 1);
        QCOMPARE(candidates[0].username, QString("EssPlayer"));
        QCOMPARE(candidates[0].typeName, QString("Microsoft"));
        // UUID dashes must be stripped for the Minecraft profile id.
        QCOMPARE(candidates[0].account->profileId(), QString("069a79f444e94726a5befca90e38aaf5"));
        QCOMPARE(candidates[0].account->saveToJson().value("msa-client-id").toString(), kEssentialClientId);
    }

    void test_essential_legacy_refresh_string()
    {
        QTemporaryDir dir;
        // Older Essential files store refreshToken as a bare string.
        QByteArray json = R"({
            "accounts": [
                { "uuid": "069a79f4-44e9-4726-a5be-fca90e38aaf5", "name": "LegacyEss",
                  "auth": { "refreshToken": "BARE_REFRESH",
                            "profile": { "id": "069a79f4-44e9-4726-a5be-fca90e38aaf5", "name": "LegacyEss" } } }
            ]
        })";
        QString path = writeTemp(dir, "microsoft_accounts.json", json);

        QString source, error;
        auto candidates = AccountImport::parseFile(path, source, error);
        QCOMPARE(source, QString("Essential"));
        QCOMPARE(candidates.size(), 1);
        QCOMPARE(candidates[0].username, QString("LegacyEss"));
    }

    void test_unsupported()
    {
        QTemporaryDir dir;
        QString path = writeTemp(dir, "feather.json", R"({ "encrypted": "blob", "not": "accounts" })");

        QString source, error;
        auto candidates = AccountImport::parseFile(path, source, error);

        QVERIFY(candidates.isEmpty());
        QVERIFY(!error.isEmpty());
    }

    void test_invalid_json()
    {
        QTemporaryDir dir;
        QString path = writeTemp(dir, "broken.json", "not json at all {{{");

        QString source, error;
        auto candidates = AccountImport::parseFile(path, source, error);

        QVERIFY(candidates.isEmpty());
        QVERIFY(!error.isEmpty());
    }
};

QTEST_GUILESS_MAIN(AccountImportTest)

#include "AccountImport_test.moc"
