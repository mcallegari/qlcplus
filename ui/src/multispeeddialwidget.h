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

#ifndef MULTISPEEDDIALWIDGET_H
#define MULTISPEEDDIALWIDGET_H

#include <QWidget>

class SpeedDial;
class QGroupBox;
class QLineEdit;

/** @addtogroup ui
 * @{
 */

class MultiSpeedDialWidget : public QWidget
{
    Q_OBJECT

public:
    MultiSpeedDialWidget(quint32 size, QWidget* parent);
    virtual ~MultiSpeedDialWidget();

    /************************************************************************
     * Speed settings
     ************************************************************************/
public:
    void setColumnTitle(quint32 idx);

    void setFadeInTitle(quint32 idx, const QString& title);
    void setFadeInEnabled(quint32 idx, bool set);
    void setFadeInVisible(quint32 idx, bool set);
    void setFadeIn(quint32 idx, int ms);
    int fadeIn(quint32 idx) const;

    void setFadeOutTitle(quint32 idx, const QString& title);
    void setFadeOutEnabled(quint32 idx, bool set);
    void setFadeOutVisible(quint32 idx, bool set);
    void setFadeOut(quint32 idx, int ms);
    int fadeOut(quint32 idx) const;

    void setHoldTitle(quint32 idx, const QString& title);
    void setHoldEnabled(quint32 idx, bool set);
    void setHoldVisible(quint32 idx, bool set);
    void setHold(quint32 idx, int ms);
    int hold(quint32 idx) const;

signals:
    void fadeInChanged(int idx, int ms);
    void fadeInTapped(int idx);

    void fadeOutChanged(int idx, int ms);
    void fadeOutTapped(int idx);

    void holdChanged(int idx, int ms);
    void holdTapped(int idx);

private slots:
    void slotFadeInChanged(int idx);
    void slotFadeOutChanged(int idx);
    void slotHoldChanged(int idx);

protected:
    QVector<SpeedDial*> m_fadeIn;
    QVector<SpeedDial*> m_fadeOut;
    QVector<SpeedDial*> m_hold;

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

