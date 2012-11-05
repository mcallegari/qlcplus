/*
  Q Light Controller
  docbrowser.h

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

#ifndef DOCBROWSER_H
#define DOCBROWSER_H

#include <QTextBrowser>
#include <QMainWindow>
#include <QTime>

class QToolBar;
class QAction;

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
    QTime m_hysteresis;
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

private:
    static DocBrowser* s_instance;
    QLCTextBrowser* m_browser;
    QToolBar* m_toolbar;

    QAction* m_backwardAction;
    QAction* m_forwardAction;
    QAction* m_homeAction;
    QAction* m_aboutQtAction;
};

#endif
