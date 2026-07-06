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

#include <QDialog>

/**
 * Scans all instances and reports ways two accounts could be linked to the
 * same operator: instances sharing an account, sharing a proxy, or exposing
 * a real IP / a leakable account. Read-only.
 */
class LinkabilityReportDialog : public QDialog {
    Q_OBJECT
   public:
    explicit LinkabilityReportDialog(QWidget* parent = nullptr);
};
