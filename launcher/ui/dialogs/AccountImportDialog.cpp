// SPDX-License-Identifier: GPL-3.0-only
/*
 *  FLauncher - Minecraft Launcher
 *  Copyright (C) 2026 FLauncher Contributors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 */

#include "AccountImportDialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>

AccountImportDialog::AccountImportDialog(const QString& sourceName,
                                         const QList<AccountImport::Candidate>& candidates,
                                         QWidget* parent)
    : QDialog(parent), m_candidates(candidates)
{
    setWindowTitle(tr("Import Accounts"));
    resize(420, 360);

    auto* layout = new QVBoxLayout(this);

    auto* header = new QLabel(tr("Found %n account(s) in the %1 file. Choose which to import:", "", candidates.size()).arg(sourceName), this);
    header->setWordWrap(true);
    layout->addWidget(header);

    m_list = new QListWidget(this);
    for (const auto& candidate : m_candidates) {
        auto* item = new QListWidgetItem(m_list);
        QString label = QStringLiteral("%1  (%2)").arg(candidate.username, candidate.typeName);
        if (!candidate.note.isEmpty())
            label += QStringLiteral(" — %1").arg(candidate.note);
        item->setText(label);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    }
    layout->addWidget(m_list, 1);

    auto* selectRow = new QHBoxLayout();
    auto* allButton = new QPushButton(tr("Select All"), this);
    auto* noneButton = new QPushButton(tr("Select None"), this);
    selectRow->addWidget(allButton);
    selectRow->addWidget(noneButton);
    selectRow->addStretch(1);
    layout->addLayout(selectRow);

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttons->button(QDialogButtonBox::Ok)->setText(tr("Import"));
    layout->addWidget(m_buttons);

    connect(allButton, &QPushButton::clicked, this, &AccountImportDialog::selectAll);
    connect(noneButton, &QPushButton::clicked, this, &AccountImportDialog::selectNone);
    connect(m_list, &QListWidget::itemChanged, this, &AccountImportDialog::updateButtons);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    updateButtons();
}

QList<AccountImport::Candidate> AccountImportDialog::selectedAccounts() const
{
    QList<AccountImport::Candidate> selected;
    for (int i = 0; i < m_list->count(); i++) {
        if (m_list->item(i)->checkState() == Qt::Checked)
            selected.append(m_candidates.at(i));
    }
    return selected;
}

void AccountImportDialog::updateButtons()
{
    bool anyChecked = false;
    for (int i = 0; i < m_list->count(); i++) {
        if (m_list->item(i)->checkState() == Qt::Checked) {
            anyChecked = true;
            break;
        }
    }
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(anyChecked);
}

void AccountImportDialog::selectAll()
{
    for (int i = 0; i < m_list->count(); i++)
        m_list->item(i)->setCheckState(Qt::Checked);
}

void AccountImportDialog::selectNone()
{
    for (int i = 0; i < m_list->count(); i++)
        m_list->item(i)->setCheckState(Qt::Unchecked);
}
