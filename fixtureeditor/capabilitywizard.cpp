/*
  Q Light Controller - Fixture Definition Editor
  capabilitywizard.cpp

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QSettings>
#include <QTextEdit>
#include <QSpinBox>
#include <QDialog>

#include "capabilitywizard.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "util.h"

#define KSettingsGeometry "capabilitywizard/geometry"

CapabilityWizard::CapabilityWizard(QWidget* parent, const QLCChannel* channel)
    : QDialog(parent)
{
    Q_ASSERT(channel != NULL);
    m_channel = channel;

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    m_nameEdit->setValidator(CAPS_VALIDATOR(this));
    slotCreateCapabilities();

    QSettings settings;
    QVariant var = settings.value(KSettingsGeometry);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
}

CapabilityWizard::~CapabilityWizard()
{
    QSettings settings;
    settings.setValue(KSettingsGeometry, saveGeometry());

    foreach (QLCCapability* cap, m_caps)
    delete cap;
}

void CapabilityWizard::slotCreateCapabilities()
{
    int start = m_startSpin->value();
    int gap = m_gapSpin->value();
    int amount = m_amountSpin->value();
    QString name = m_nameEdit->text();
    uchar min = start;
    uchar max = min + gap;
    QLCCapability* cap;

    /* Destroy existing capabilities */
    foreach (QLCCapability* cap, m_caps)
    delete cap;
    m_caps.clear();

    /* Create new capabilities */
    for (int i = 0; i < amount; i++)
    {
        /* Create a name with the optional hash mark */
        QString modname(name);
        modname.replace("#", QString("%1").arg(i + 1));

        /* Create a new capability and add it to our list */
        cap = new QLCCapability(min, max, modname);
        m_caps << cap;

        /* Bail out when the maximum DMX value has been reached */
        if (max == UCHAR_MAX)
            break;

        /* Increment for the next round */
        min = max + 1;
        max = min + gap;

        /* Bail out if next round would overflow */
        if (max < min)
            break;
    }

    /* Replace capabilities in the list widget */
    m_list->clear();
    foreach (cap, m_caps)
    {
        QListWidgetItem* item;
        item = new QListWidgetItem(m_list);
        item->setText(QString("[%1 - %2] %3").arg(cap->min())
                      .arg(cap->max()).arg(cap->name()));

        if (m_channel->searchCapability(cap->min()) != NULL ||
                m_channel->searchCapability(cap->max()) != NULL)
        {
            /* Disable the item to indicate overlapping */
            item->setFlags(0);
        }
    }
}
