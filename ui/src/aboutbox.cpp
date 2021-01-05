/*
  Q Light Controller
  aboutbox.cpp

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

#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>
#include <QAction>

#include "qlcconfig.h"
#include "aboutbox.h"

AboutBox::AboutBox(QWidget* parent) : QDialog (parent)
{
    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    m_titleLabel->setText(APPNAME);
    m_versionLabel->setText(APPVERSION);
    m_copyrightLabel->setText(QString("Copyright &copy; <B>Heikki Junnila, Massimo Callegari</B> %1")
                              .arg(tr("and contributors:")));
    m_websiteLabel->setText(tr("Website: %1").arg("<A HREF=\"http://www.qlcplus.org/\">http://www.qlcplus.org/</A>"));
    connect(m_websiteLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotWebsiteClicked()));

    connect(m_contributors, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(slotItemClicked()));
    m_contributors->clear();
    m_contributors->addItem("Contributors:");
    m_contributors->addItem("Jano Svitok");
    m_contributors->addItem("David Garyga");
    m_contributors->addItem(QString::fromUtf8("Lukas Jähn"));
    m_contributors->addItem("Robert Box");
    m_contributors->addItem("Thomas Achtner");
    m_contributors->addItem("Joep Admiraal");
    m_contributors->addItem("Oliver Ruempelein");
    m_contributors->addItem("Jannis Achstetter");
    m_contributors->addItem("Stefan Riemens");
    m_contributors->addItem("Florian Euchner");
    m_contributors->addItem("Bartosz Grabias");
    m_contributors->addItem("NiKoyes");
    m_contributors->addItem("Heiko Fanieng");
    m_contributors->addItem("Raymond Van Laake");
    m_contributors->addItem(QString::fromUtf8("Luis García Tornel"));
    m_contributors->addItem("Jan Lachman");
    m_contributors->addItem("Nuno Almeida");
    m_contributors->addItem("Santiago Benejam Torres");
    m_contributors->addItem(QString::fromUtf8("Jérôme Lebleu"));
    m_contributors->addItem("Koichiro Saito");
    m_contributors->addItem("Karri Kaksonen");
    m_contributors->addItem("Stefan Krupop");
    m_contributors->addItem("Nathan Durnan");
    m_contributors->addItem("Giorgio Rebecchi");
    m_contributors->addItem("Klaus Weidenbach");
    m_contributors->addItem("Stefan Krumm");
    m_contributors->addItem(QString::fromUtf8("Christian Sühs"));
    m_contributors->addItem("Simon Newton");
    m_contributors->addItem("Christopher Staite");
    m_contributors->addItem("Lutz Hillebrand");
    m_contributors->addItem("Matthew Jaggard");
    m_contributors->addItem("Ptit Vachon");
    m_contributors->addItem("Tolmino");

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
    m_row = -1;
    m_increment = 1;
    m_timer->start(500);
}

AboutBox::~AboutBox()
{
}

void AboutBox::slotTimeout()
{
    if (m_row <= 0)
        m_increment = 1;
    else if (m_row >= m_contributors->count())
        m_increment = -1;

    m_row += m_increment;
    m_contributors->scrollToItem(m_contributors->item(m_row));
}

void AboutBox::slotItemClicked()
{
    if (m_timer != NULL)
    {
        m_timer->stop();
        delete m_timer;
        m_timer = NULL;
    }
}

void AboutBox::slotWebsiteClicked()
{
    QDesktopServices::openUrl(QUrl("http://www.qlcplus.org/"));
}
