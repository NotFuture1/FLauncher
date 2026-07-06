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
#include <QList>

#include "minecraft/auth/AccountImport.h"

class QListWidget;
class QDialogButtonBox;
class QLabel;

/**
 * Lets the user choose which of the accounts found in an imported file to add.
 */
class AccountImportDialog : public QDialog {
    Q_OBJECT
   public:
    explicit AccountImportDialog(const QString& sourceName, const QList<AccountImport::Candidate>& candidates, QWidget* parent = nullptr);

    //! Accounts the user ticked. Only valid after exec() returns Accepted.
    QList<AccountImport::Candidate> selectedAccounts() const;

   private slots:
    void updateButtons();
    void selectAll();
    void selectNone();

   private:
    QList<AccountImport::Candidate> m_candidates;
    QListWidget* m_list;
    QDialogButtonBox* m_buttons;
};
