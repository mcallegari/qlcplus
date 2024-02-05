/*
  Q Light Controller
  docbrowser.cpp

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

#include <QDesktopServices>
#include <QGestureEvent>
#include <QSwipeGesture>
#include <QApplication>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QToolBar>
#include <QScreen>
#include <QAction>
#include <QDebug>
#include <QIcon>
#include <QUrl>

#include "docbrowser.h"
#include "qlcconfig.h"
#include "qlcfile.h"
#include "apputil.h"

#define SETTINGS_GEOMETRY "documentbrowser/geometry"
#define HYSTERESIS_MS 100

/****************************************************************************
 * QLCTextBrowser
 ****************************************************************************/

QLCTextBrowser::QLCTextBrowser(QWidget* parent)
    : QTextBrowser(parent)
{
    grabGesture(Qt::SwipeGesture);
    m_hysteresis.start();
}

QLCTextBrowser::~QLCTextBrowser()
{
}

bool QLCTextBrowser::event(QEvent* ev)
{
    if (ev->type() == QEvent::Gesture)
    {
        QGestureEvent* gesture = static_cast<QGestureEvent*> (ev);
        QSwipeGesture* swipe = qobject_cast<QSwipeGesture*> (
            gesture->gesture(Qt::SwipeGesture));
        if (swipe == NULL)
        {
            /* NOP */
        }
        else if (swipe->horizontalDirection() == QSwipeGesture::Left)
        {
            if (m_hysteresis.elapsed() > HYSTERESIS_MS)
            {
                backward();
                ev->accept();
                m_hysteresis.start();
            }
        }
        else if (swipe->horizontalDirection() == QSwipeGesture::Right)
        {
            if (m_hysteresis.elapsed() > HYSTERESIS_MS)
            {
                forward();
                ev->accept();
                m_hysteresis.start();
            }
        }
    }

    return QTextBrowser::event(ev);
}

/****************************************************************************
 * DocBrowser
 ****************************************************************************/
DocBrowser* DocBrowser::s_instance = NULL;

DocBrowser::DocBrowser(QWidget* parent)
    : QWidget(parent, Qt::Window)
    , m_backwardAction(NULL)
    , m_forwardAction(NULL)
    , m_homeAction(NULL)
    , m_aboutQtAction(NULL)
{
    new QVBoxLayout(this);

    setWindowTitle(tr("%1 - Document Browser").arg(APPNAME));
    setWindowIcon(QIcon(":/help.png"));

    /* Recall window size */
    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
    {
        restoreGeometry(var.toByteArray());
    }
    else
    {
        QScreen *screen = QGuiApplication::screens().first();
        QRect rect = screen->availableGeometry();
        int rWd = rect.width() / 4;
        int rHd = rect.height() / 4;
        resize(rWd * 3, rHd * 3);
        move(rWd / 2, rHd / 2);
    }
    AppUtil::ensureWidgetIsVisible(this);

    /* Actions */
    m_backwardAction = new QAction(QIcon(":/back.png"), tr("Backward"), this);
    m_forwardAction = new QAction(QIcon(":/forward.png"), tr("Forward"), this);
    m_homeAction = new QAction(QIcon(":/qlcplus.png"), tr("Index"), this);
    m_aboutQtAction = new QAction(QIcon(":/qt.png"), tr("About Qt"), this);
    m_closeAction = new QAction(QIcon(":/delete.png"), tr("Close this window"), this);

    m_backwardAction->setEnabled(false);
    m_forwardAction->setEnabled(false);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(close()));
    addAction(action);

    /* Toolbar */
    m_toolbar = new QToolBar("Document Browser", this);
    layout()->addWidget(m_toolbar);
    m_toolbar->addAction(m_backwardAction);
    m_toolbar->addAction(m_forwardAction);
    m_toolbar->addAction(m_homeAction);
    QWidget* widget = new QWidget(this);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_toolbar->addWidget(widget);
    m_toolbar->addAction(m_aboutQtAction);

    /* Browser */
    m_browser = new QLCTextBrowser(this);
    // Override the link handler to only assume http/https are external links
    // This may be replaced with setOpenExternalLinks(true) and slotAnchorClicked removed if
    // Qt fixes setOpenExternalLinks to work with relative URL references
    m_browser->setOpenLinks(false);
    layout()->addWidget(m_browser);

    connect(m_browser, SIGNAL(backwardAvailable(bool)),
            this, SLOT(slotBackwardAvailable(bool)));
    connect(m_browser, SIGNAL(forwardAvailable(bool)),
            this, SLOT(slotForwardAvailable(bool)));
    connect(m_backwardAction, SIGNAL(triggered(bool)),
            m_browser, SLOT(backward()));
    connect(m_forwardAction, SIGNAL(triggered(bool)),
            m_browser, SLOT(forward()));
    connect(m_homeAction, SIGNAL(triggered(bool)),
            m_browser, SLOT(home()));
    connect(m_aboutQtAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAboutQt()));
    connect(m_browser, SIGNAL(anchorClicked(QUrl)), this, SLOT(slotAnchorClicked(QUrl)));
    if (QLCFile::hasWindowManager() == false)
    {
        m_toolbar->addAction(m_closeAction);
        connect(m_closeAction, SIGNAL(triggered(bool)),
                this, SLOT(slotCloseWindow()));
    }

    /* Set document search paths */
    QStringList searchPaths;
    searchPaths << QLCFile::systemDirectory(QString("%1/html/").arg(DOCSDIR)).path();

    m_browser->setSearchPaths(searchPaths);
    m_browser->setSource(QUrl("file:index.html"));
}

DocBrowser::~DocBrowser()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
    s_instance = NULL;
}

void DocBrowser::slotAnchorClicked(QUrl url){
    if (url.scheme() == QLatin1String("http") || url.scheme() == QLatin1String("https"))
     {
        QDesktopServices::openUrl(url);
     }
     else
     {
        m_browser->setSource(url);
     }
}

void DocBrowser::createAndShow(QWidget* parent)
{
    if (s_instance == NULL)
    {
        s_instance = new DocBrowser(parent);
        s_instance->setAttribute(Qt::WA_DeleteOnClose);
    }

    s_instance->show();
    s_instance->raise();
}

void DocBrowser::slotBackwardAvailable(bool available)
{
    m_backwardAction->setEnabled(available);
}

void DocBrowser::slotForwardAvailable(bool available)
{
    m_forwardAction->setEnabled(available);
}

void DocBrowser::slotAboutQt()
{
    QMessageBox::aboutQt(this, QString(APPNAME));
}

void DocBrowser::slotCloseWindow()
{
    close();
}
