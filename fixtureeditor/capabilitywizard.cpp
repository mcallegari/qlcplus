/*
  Q Light Controller - Fixture Definition Editor
  capabilitywizard.cpp

  Copyright (C) Heikki Junnila

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QSettings>
#include <QTextEdit>
#include <QSpinBox>
#include <QDialog>
#include <QAction>

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
    int width = m_widthSpin->value();
    int amount = m_amountSpin->value();
    QString name = m_nameEdit->text();
    uchar min = start;
    uchar max = min + width - 1;
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
        max = min + width - 1;

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
            item->setFlags(Qt::NoItemFlags);
        }
    }
}
