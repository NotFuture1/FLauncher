// SPDX-License-Identifier: GPL-3.0-only
/*
 *  FLauncher - Minecraft Launcher
 *  Copyright (C) 2026 FLauncher Contributors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 */

#include "LinkabilityReportDialog.h"

#include <QDialogButtonBox>
#include <QHash>
#include <QLabel>
#include <QStringList>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "Application.h"
#include "InstanceList.h"
#include "minecraft/auth/AccountList.h"
#include "settings/SettingsObject.h"

namespace {
QString escape(const QString& in)
{
    return in.toHtmlEscaped();
}
}  // namespace

LinkabilityReportDialog::LinkabilityReportDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Linkability Report"));
    resize(560, 460);

    auto* layout = new QVBoxLayout(this);
    auto* intro = new QLabel(tr("Ways your accounts could be linked to the same person. Fix the red items first."), this);
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto* view = new QTextBrowser(this);
    layout->addWidget(view, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttons);

    auto* instances = APPLICATION->instances();
    auto* accounts = APPLICATION->accounts();

    // Gather per-instance identity: bound account and proxy.
    QHash<QString, QStringList> accountToInstances;  // profileId -> instance names
    QHash<QString, QStringList> proxyToInstances;     // host:port -> instance names
    QStringList unboundInstances;
    QStringList proxylessInstances;

    for (int i = 0; i < instances->count(); i++) {
        auto* inst = instances->at(i);
        auto settings = inst->settings();
        const QString name = inst->name();

        if (settings->get("UseAccountForInstance").toBool()) {
            const QString profileId = settings->get("InstanceAccountId").toString();
            if (!profileId.isEmpty())
                accountToInstances[profileId] << name;
        } else {
            unboundInstances << name;
        }

        if (settings->get("UseProxyForInstance").toBool()) {
            const QString host = settings->get("InstanceProxyHost").toString();
            const int port = settings->get("InstanceProxyPort").toInt();
            if (!host.isEmpty() && port > 0)
                proxyToInstances[QStringLiteral("%1:%2").arg(host).arg(port)] << name;
            else
                proxylessInstances << name;
        } else {
            proxylessInstances << name;
        }
    }

    QStringList html;

    // Critical: the same account on more than one instance.
    QStringList critical;
    for (auto it = accountToInstances.constBegin(); it != accountToInstances.constEnd(); ++it) {
        if (it.value().size() > 1) {
            const int idx = accounts->findAccountByProfileId(it.key());
            const QString accName = idx == -1 ? tr("(unknown account)") : accounts->at(idx)->profileName();
            critical << tr("Account <b>%1</b> is used by multiple instances: %2")
                            .arg(escape(accName), escape(it.value().join(", ")));
        }
    }

    // Warning: instances that share the exact same proxy exit.
    QStringList warnings;
    for (auto it = proxyToInstances.constBegin(); it != proxyToInstances.constEnd(); ++it) {
        if (it.value().size() > 1)
            warnings << tr("These instances share the same proxy (same exit IP): %1").arg(escape(it.value().join(", ")));
    }

    // Info: unprotected instances.
    QStringList info;
    if (!unboundInstances.isEmpty())
        info << tr("Not locked to an account (could launch with the wrong account): %1").arg(escape(unboundInstances.join(", ")));
    if (!proxylessInstances.isEmpty())
        info << tr("No proxy set (connects with your real IP): %1").arg(escape(proxylessInstances.join(", ")));

    auto section = [&](const QString& color, const QString& title, const QStringList& items) {
        if (items.isEmpty())
            return;
        html << QStringLiteral("<p style='color:%1'><b>%2</b></p><ul>").arg(color, escape(title));
        for (const QString& item : items)
            html << QStringLiteral("<li>%1</li>").arg(item);
        html << QStringLiteral("</ul>");
    };

    section("#e05555", tr("Critical"), critical);
    section("#e0a000", tr("Warning"), warnings);
    section("#7f7f7f", tr("For your information"), info);

    if (critical.isEmpty() && warnings.isEmpty() && info.isEmpty())
        html << QStringLiteral("<p>%1</p>").arg(tr("No linkability issues found across your instances."));

    view->setHtml(html.join('\n'));
}
