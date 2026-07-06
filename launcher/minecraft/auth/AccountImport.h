// SPDX-License-Identifier: GPL-3.0-only
/*
 *  FLauncher - Minecraft Launcher
 *  Copyright (C) 2026 FLauncher Contributors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 */

#pragma once

#include <QList>
#include <QString>

#include "minecraft/auth/MinecraftAccount.h"

/**
 * Import Minecraft accounts exported by other launchers.
 *
 * Supported sources: FLauncher/Prism Launcher, MultiMC, Essential, Feather.
 * Each source's accounts file is parsed into a list of ready-to-add
 * MinecraftAccount objects, which the caller can then let the user pick from.
 */
namespace AccountImport {

/// A single account discovered inside an imported file.
struct Candidate {
    MinecraftAccountPtr account;  //!< fully-formed account, ready to add to the list
    QString username;             //!< profile / display name shown to the user
    QString typeName;             //!< "Microsoft" or "Offline"
    QString note;                 //!< optional caveat, e.g. "may need to sign in again"
};

/**
 * Parse an accounts file exported by a supported launcher.
 *
 * @param path       path to the JSON file the user selected
 * @param sourceName set to a human-readable launcher name on success (e.g. "MultiMC")
 * @param errorOut   set to a human-readable message on failure
 * @return the accounts found in the file, or an empty list on failure
 */
QList<Candidate> parseFile(const QString& path, QString& sourceName, QString& errorOut);

}  // namespace AccountImport
