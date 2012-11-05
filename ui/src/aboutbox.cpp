/*
  Q Light Controller
  aboutbox.cpp

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

#include <QDebug>
#include <QLabel>
#include <QTimer>

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
    m_copyrightLabel->setText(QString("Copyright &copy; <B>Heikki Junnila</B> %1")
                              .arg(tr("and contributors:")));
    m_websiteLabel->setText(tr("Website: %1").arg("<A HREF=\"http://qlc.sourceforge.net\">http://qlc.sourceforge.net</A>"));

    connect(m_contributors, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(slotItemClicked()));
    m_contributors->clear();
    m_contributors->addItem("janek3");
    m_contributors->addItem("Klaus Weidenbach");
    m_contributors->addItem("Stefan Krumm");
    m_contributors->addItem(QByteArray::fromPercentEncoding("Christian S%fchs"));
    m_contributors->addItem("Simon Newton");
    m_contributors->addItem("Christopher Staite");
    m_contributors->addItem("Lutz Hillebrand");
    m_contributors->addItem("Matthew Jaggard");
    m_contributors->addItem("Ptit Vachon");
    m_contributors->addItem("NiKoyes");
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
