/*
  Q Light Controller
  launcher.cpp

  Copyright (c) Heikki Junnila

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

#include <QFileOpenEvent>
#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QProcess>
#include <QPixmap>
#include <QLabel>
#include <QUrl>

#include <unistd.h>

#include "qlcconfig.h"
#include "qlcfile.h"

#include "launcher.h"

Launcher::Launcher(QWidget* parent) : QWidget(parent)
{
    QGridLayout* lay;
    lay = new QGridLayout(this);
    setLayout(lay);

    setWindowTitle(APPNAME);

    QLabel* icon = new QLabel(this);
    icon->setPixmap(QIcon(":/qlcplus.png").pixmap(64));
    lay->addWidget(icon, 0, 0, 1, 1);

    QString text("<H1>%1 %2</H1>");
    QLabel* title = new QLabel(text.arg(APPNAME).arg(APPVERSION), this);
    lay->addWidget(title, 0, 1, 1, 2);

    QPushButton* fxed = new QPushButton(FXEDNAME, this);
    fxed->setToolTip(tr("Launch %1").arg(FXEDNAME));
    connect(fxed, SIGNAL(clicked()), this, SLOT(slotFXEDClicked()));
    lay->addWidget(fxed, 1, 1, 1, 1);

    QPushButton* qlc = new QPushButton(APPNAME, this);
    qlc->setToolTip(tr("Launch the main %1 application").arg(APPNAME));
    connect(qlc, SIGNAL(clicked()), this, SLOT(slotQLCClicked()));
    lay->addWidget(qlc, 1, 2, 1, 1);
}

Launcher::~Launcher()
{
}

void Launcher::slotFXEDClicked()
{
    launchFXED(QApplication::arguments());
}

void Launcher::slotQLCClicked()
{
    launchQLC(QApplication::arguments());
}

void Launcher::launchFXED(const QStringList& arguments)
{
    QString path(QApplication::applicationDirPath());
    if (path.endsWith(QString("/")) == false)
        path += QString("/");
    path += QString("qlcplus-fixtureeditor");
    QProcess::startDetached(path, arguments);
    QApplication::exit();
}

void Launcher::launchQLC(const QStringList& arguments)
{
    QString path(QApplication::applicationDirPath());
    if (path.endsWith(QString("/")) == false)
        path += QString("/");
    path += QString("qlcplus");
    QProcess::startDetached(path, arguments);
    QApplication::exit();
}

bool Launcher::eventFilter(QObject* object, QEvent* event)
{
    bool retval = false;

    // Not interested in other objects' events
    if (object != QApplication::instance())
        return false;

    if (event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent* fileOpenEvent(static_cast<QFileOpenEvent*>(event));
        QString path(fileOpenEvent->url().path());
        if (path.isEmpty() == true)
        {
            path = fileOpenEvent->file();
            if (path.isEmpty() == true)
            {
                // Nothing to open
                event->ignore();
                return false;
            }
        }

        if (path.endsWith(KExtWorkspace, Qt::CaseInsensitive) == true)
        {
            launchQLC(QApplication::arguments() << "--open" << path);
            retval = true;
        }
        else if (path.endsWith(KExtFixture, Qt::CaseInsensitive) == true)
        {
            launchFXED(QApplication::arguments() << "--open" << path);
            retval = true;
        }
    }

    if (retval == true)
        event->accept();
    else
        event->ignore();
    return retval;
}
