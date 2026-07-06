// SPDX-License-Identifier: GPL-3.0-only
/*
 *  FLauncher - Minecraft Launcher
 *  Copyright (C) 2026 FLauncher Contributors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 */

#include "minecraft/auth/AccountImport.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QObject>

namespace {

// The Microsoft OAuth client IDs each launcher mints its refresh tokens under.
// A refresh token only works with the client ID that created it, so an imported
// account must remember which one to use when it later refreshes.
const QString kMultiMCClientId = QStringLiteral("499546d9-bbfe-4b9b-a086-eb3d75afb78f");
const QString kEssentialClientId = QStringLiteral("e39cc675-eb52-4475-b5f8-82aaae14eeba");

QString stripDashes(QString uuid)
{
    return uuid.remove('-');
}

AccountImport::Candidate makeCandidate(MinecraftAccountPtr account, const QString& username, bool isOffline)
{
    AccountImport::Candidate candidate;
    candidate.account = account;
    candidate.username = username.isEmpty() ? QObject::tr("(unknown)") : username;
    candidate.typeName = isOffline ? QObject::tr("Offline") : QObject::tr("Microsoft");
    return candidate;
}

// Prism Launcher / MultiMC share the same "formatVersion 3" account layout.
// MultiMC omits "msa-client-id", so inject its client ID so refresh keeps working.
QList<AccountImport::Candidate> parseV3Accounts(const QJsonArray& accounts, bool isMultiMC)
{
    QList<AccountImport::Candidate> result;
    for (const QJsonValue& value : accounts) {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();

        QString type = obj.value("type").toString();
        bool isOffline = (type == "Offline");

        if (isMultiMC && !isOffline && !obj.contains("msa-client-id"))
            obj["msa-client-id"] = kMultiMCClientId;

        MinecraftAccountPtr account = MinecraftAccount::loadFromJsonV3(obj);
        if (!account)
            continue;

        QString username = obj.value("profile").toObject().value("name").toString();
        result.append(makeCandidate(account, username, isOffline));
    }
    return result;
}

// Essential stores accounts in its own shape; convert each into the v3 layout.
QList<AccountImport::Candidate> parseEssentialAccounts(const QJsonArray& accounts)
{
    QList<AccountImport::Candidate> result;
    for (const QJsonValue& value : accounts) {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();

        QString name = obj.value("name").toString();
        QString uuid = obj.value("uuid").toString();
        QJsonObject auth = obj.value("auth").toObject();

        // refreshToken may be a bare string (legacy) or a { value, expires } object.
        QString refreshToken;
        QJsonValue refreshVal = auth.value("refreshToken");
        if (refreshVal.isString())
            refreshToken = refreshVal.toString();
        else if (refreshVal.isObject())
            refreshToken = refreshVal.toObject().value("value").toString();

        // Without a refresh token the account cannot be used or re-authenticated.
        if (refreshToken.isEmpty())
            continue;

        QJsonObject v3;
        v3["type"] = "MSA";
        v3["msa-client-id"] = kEssentialClientId;

        QJsonObject msa;
        msa["refresh_token"] = refreshToken;
        QString msaAccess = auth.value("accessToken").toObject().value("value").toString();
        if (!msaAccess.isEmpty())
            msa["token"] = msaAccess;
        v3["msa"] = msa;

        // Carry the short-lived Minecraft token too, so the account works immediately
        // instead of only after its first refresh.
        QString mcAccess = obj.value("accessToken").toString();
        if (!mcAccess.isEmpty()) {
            QJsonObject ygg;
            ygg["token"] = mcAccess;
            v3["ygg"] = ygg;
        }

        // profileFromJSONV3 rejects a profile that lacks a skin object or capes
        // array, so build a complete (if blank) profile the parser will accept.
        QJsonObject skin;
        skin["id"] = "";
        skin["url"] = "";
        skin["variant"] = "";

        QJsonObject profile;
        profile["id"] = stripDashes(uuid);
        profile["name"] = name;
        profile["skin"] = skin;
        profile["capes"] = QJsonArray();
        v3["profile"] = profile;

        MinecraftAccountPtr account = MinecraftAccount::loadFromJsonV3(v3);
        if (!account)
            continue;

        result.append(makeCandidate(account, name, false));
    }
    return result;
}

bool looksLikeEssential(const QJsonArray& accounts)
{
    for (const QJsonValue& value : accounts) {
        if (value.isObject() && value.toObject().contains("auth"))
            return true;
    }
    return false;
}

bool looksLikePrism(const QJsonArray& accounts)
{
    for (const QJsonValue& value : accounts) {
        if (value.isObject() && value.toObject().contains("msa-client-id"))
            return true;
    }
    return false;
}

}  // namespace

namespace AccountImport {

QList<Candidate> parseFile(const QString& path, QString& sourceName, QString& errorOut)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        errorOut = QObject::tr("Could not open the file: %1").arg(file.errorString());
        return {};
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        errorOut = QObject::tr(
            "This file isn't a supported accounts file.\n\n"
            "Supported: Prism Launcher, MultiMC, and Essential accounts files. "
            "Feather stores its accounts encrypted and cannot be imported.");
        return {};
    }

    QJsonObject root = doc.object();

    QJsonArray accounts;
    if (root.value("accounts").isArray()) {
        accounts = root.value("accounts").toArray();
    } else if (root.contains("type")) {
        // A single per-account file (e.g. one of FLauncher's own account files).
        accounts.append(root);
    } else {
        errorOut = QObject::tr(
            "This file isn't a supported accounts file.\n\n"
            "Supported: Prism Launcher, MultiMC, and Essential accounts files. "
            "Feather stores its accounts encrypted and cannot be imported.");
        return {};
    }

    QList<Candidate> candidates;
    if (looksLikeEssential(accounts)) {
        sourceName = QObject::tr("Essential");
        candidates = parseEssentialAccounts(accounts);
    } else if (looksLikePrism(accounts)) {
        sourceName = QObject::tr("Prism Launcher");
        candidates = parseV3Accounts(accounts, /*isMultiMC=*/false);
    } else {
        // Prism-style layout without the Prism-only client-ID field: treat as MultiMC.
        sourceName = QObject::tr("MultiMC");
        candidates = parseV3Accounts(accounts, /*isMultiMC=*/true);
    }

    if (candidates.isEmpty())
        errorOut = QObject::tr("No importable accounts were found in this file.");

    return candidates;
}

}  // namespace AccountImport
