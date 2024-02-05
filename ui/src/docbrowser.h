/*
  Q Light Controller
  docbrowser.h

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

#ifndef DOCBROWSER_H
#define DOCBROWSER_H

#include <QTextBrowser>
#include <QPushButton>
#include <QMainWindow>
#include <QElapsedTimer>
#include <QUrl>

class QToolBar;
class QAction;

/** @addtogroup ui UI
 * @{
 */

/****************************************************************************
 * QLCTextBrowser
 ****************************************************************************/

class QLCTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    QLCTextBrowser(QWidget* parent);
    virtual ~QLCTextBrowser();

protected:
    bool event(QEvent* ev);

private:
    QElapsedTimer m_hysteresis;
};

/****************************************************************************
 * DocBrowser
 ****************************************************************************/

class DocBrowser : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(DocBrowser)

private:
    DocBrowser(QWidget* parent);

public:
    static void createAndShow(QWidget* parent);
    ~DocBrowser();

private slots:
    void slotBackwardAvailable(bool);
    void slotForwardAvailable(bool);
    void slotAboutQt();
    void slotCloseWindow();
    void slotAnchorClicked(QUrl);

private:
    static DocBrowser* s_instance;
    QLCTextBrowser* m_browser;
    QToolBar* m_toolbar;

    QAction* m_backwardAction;
    QAction* m_forwardAction;
    QAction* m_homeAction;
    QAction* m_aboutQtAction;
    QAction* m_closeAction;
};

/** @} */

#endif
