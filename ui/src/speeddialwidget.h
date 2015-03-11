/*
  Q Light Controller
  speeddialwidget.h

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

#ifndef SPEEDDIALWIDGET_H
#define SPEEDDIALWIDGET_H

#include <QWidget>

class SpeedDial;
class QGroupBox;
class QLineEdit;

/** @addtogroup ui
 * @{
 */

#define SPEED_DIAL_FLAGS Qt::WindowFlags( \
    (Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Window | Qt::WindowStaysOnTopHint | \
     Qt::WindowMinimizeButtonHint) & (~Qt::WindowCloseButtonHint))

class SpeedDialWidget : public QWidget
{
    Q_OBJECT

public:
    SpeedDialWidget(QWidget* parent);
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

/** @} */

#endif
