/*
  Q Light Controller
  speeddialwidget.h

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

#ifndef SPEEDDIALWIDGET_H
#define SPEEDDIALWIDGET_H

#include <QWidget>

class SpeedDial;
class QGroupBox;
class QLineEdit;

#define SPEED_DIAL_FLAGS Qt::WindowFlags( \
    (Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Window | Qt::WindowStaysOnTopHint | \
     Qt::WindowMinimizeButtonHint) & (~Qt::WindowCloseButtonHint))

class SpeedDialWidget : public QWidget
{
    Q_OBJECT

public:
    SpeedDialWidget(QWidget* parent, Qt::WindowFlags = SPEED_DIAL_FLAGS);
    ~SpeedDialWidget();

    /************************************************************************
     * Speed settings
     ************************************************************************/
public:
    void setFadeInTitle(const QString& title);
    void setFadeInEnabled(bool set);
    void setFadeInVisible(bool set);
    void setFadeInSpeed(int ms);
    int fadeIn() const;

    void setFadeOutTitle(const QString& title);
    void setFadeOutEnabled(bool set);
    void setFadeOutVisible(bool set);
    void setFadeOutSpeed(int ms);
    int fadeOut() const;

    void setDurationTitle(const QString& title);
    void setDurationEnabled(bool set);
    void setDurationVisible(bool set);
    void setDuration(int ms);
    int duration() const;

signals:
    void fadeInChanged(int ms);
    void fadeInTapped();

    void fadeOutChanged(int ms);
    void fadeOutTapped();

    void holdChanged(int ms);
    void holdTapped();

private:
    SpeedDial* m_fadeIn;
    SpeedDial* m_fadeOut;
    SpeedDial* m_hold;

    /************************************************************************
     * Optional text
     ************************************************************************/
public:
    void setOptionalTextTitle(const QString& title);
    QString optionalTextTitle() const;

    void setOptionalText(const QString& text);
    QString optionalText() const;

signals:
    void optionalTextEdited(const QString& text);

private:
    QGroupBox* m_optionalTextGroup;
    QLineEdit* m_optionalTextEdit;
};

#endif
