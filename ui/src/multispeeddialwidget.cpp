/*
  Q Light Controller
  speeddialwidget.cpp

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
#include <QGroupBox>
#include <QLineEdit>
#include <QLayout>
#include <QDebug>
#include <QSignalMapper>

#include "multispeeddialwidget.h"
#include "mastertimer.h"
#include "speeddial.h"
#include "apputil.h"

#define WINDOW_FLAGS Qt::WindowFlags( \
    (Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Window | \
     Qt::WindowStaysOnTopHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint))

#define SETTINGS_GEOMETRY "speeddialwidget/geometry"
#define SETTINGS_DIRECTION "speeddialwidget/direction"

MultiSpeedDialWidget::MultiSpeedDialWidget(quint32 size, QWidget* parent)
    : QWidget(parent)
    , m_optionalTextGroup(NULL)
    , m_optionalTextEdit(NULL)
{
    setWindowFlags(WINDOW_FLAGS);

    QSettings settings;
    QBoxLayout::Direction mainDirection;
    QBoxLayout::Direction dialsDirection;

    // Load direction settings
    {
        QVariant var = settings.value(SETTINGS_DIRECTION);
        if (var.isValid() == true)
            dialsDirection = QBoxLayout::Direction(var.toInt());
        else
            dialsDirection = QBoxLayout::TopToBottom;
        switch (dialsDirection)
        {
            case QBoxLayout::LeftToRight:
            case QBoxLayout::RightToLeft:
                mainDirection = QBoxLayout::TopToBottom;
                break;
            case QBoxLayout::BottomToTop:
            case QBoxLayout::TopToBottom:
            default:
                mainDirection = QBoxLayout::LeftToRight;
                break;
        }
    }

    new QBoxLayout(QBoxLayout::TopToBottom, this);
    {
        QBoxLayout* dialsLayout = new QBoxLayout(mainDirection);

        QSignalMapper* fadeInChangedMapper = new QSignalMapper(this);
        QSignalMapper* fadeInTappedMapper = new QSignalMapper(this);
        QSignalMapper* fadeOutChangedMapper = new QSignalMapper(this);
        QSignalMapper* fadeOutTappedMapper = new QSignalMapper(this);
        QSignalMapper* holdChangedMapper = new QSignalMapper(this);
        QSignalMapper* holdTappedMapper = new QSignalMapper(this);
        for (quint32 i = 0; i < size; ++i)
        {
            /* Layout with customizable direction */
            QBoxLayout* lay = new QBoxLayout(dialsDirection);

            /* Create dials */
            m_fadeIn.push_back(new SpeedDial(this));
            m_fadeIn[i]->setTitle(tr("Fade In"));
            fadeInChangedMapper->setMapping(m_fadeIn[i], i);
            connect(m_fadeIn[i], SIGNAL(valueChanged(int)), fadeInChangedMapper, SLOT(map()));
            fadeInTappedMapper->setMapping(m_fadeIn[i], i);
            connect(m_fadeIn[i], SIGNAL(tapped()), fadeInTappedMapper, SLOT(map()));
            lay->addWidget(m_fadeIn[i]);

            m_fadeOut.push_back(new SpeedDial(this));
            m_fadeOut[i]->setTitle(tr("Fade Out"));
            fadeOutChangedMapper->setMapping(m_fadeOut[i], i);
            connect(m_fadeOut[i], SIGNAL(valueChanged(int)), fadeOutChangedMapper, SLOT(map()));
            fadeOutTappedMapper->setMapping(m_fadeOut[i], i);
            connect(m_fadeOut[i], SIGNAL(tapped()), fadeOutTappedMapper, SLOT(map()));
            lay->addWidget(m_fadeOut[i]);

            m_hold.push_back(new SpeedDial(this));
            m_hold[i]->setTitle(tr("Hold"));
            holdChangedMapper->setMapping(m_hold[i], i);
            connect(m_hold[i], SIGNAL(valueChanged(int)), holdChangedMapper, SLOT(map()));
            holdTappedMapper->setMapping(m_hold[i], i);
            connect(m_hold[i], SIGNAL(tapped()), holdTappedMapper, SLOT(map()));
            lay->addWidget(m_hold[i]);

            lay->addStretch();

            dialsLayout->addItem(lay);
        }
        connect(fadeInChangedMapper, SIGNAL(mapped(int)), this, SLOT(slotFadeInChanged(int)));
        connect(fadeInTappedMapper, SIGNAL(mapped(int)), this, SIGNAL(fadeInTapped(int)));
        connect(fadeOutChangedMapper, SIGNAL(mapped(int)), this, SLOT(slotFadeOutChanged(int)));
        connect(fadeOutTappedMapper, SIGNAL(mapped(int)), this, SIGNAL(fadeOutTapped(int)));
        connect(holdChangedMapper, SIGNAL(mapped(int)), this, SLOT(slotHoldChanged(int)));
        connect(holdTappedMapper, SIGNAL(mapped(int)), this, SIGNAL(holdTapped(int)));

        layout()->addItem(dialsLayout);
    }

    /* Optional text */
    {
        m_optionalTextGroup = new QGroupBox(this);
        new QVBoxLayout(m_optionalTextGroup);
        m_optionalTextEdit = new QLineEdit(m_optionalTextGroup);
        m_optionalTextGroup->layout()->addWidget(m_optionalTextEdit);
        m_optionalTextGroup->setVisible(false);
        connect(m_optionalTextEdit, SIGNAL(textEdited(const QString&)),
                this, SIGNAL(optionalTextEdited(const QString&)));
        layout()->addWidget(m_optionalTextGroup);
    }

    /* Position */
    {
        QVariant var = settings.value(SETTINGS_GEOMETRY);
        if (var.isValid() == true)
            this->restoreGeometry(var.toByteArray());
        AppUtil::ensureWidgetIsVisible(this);
    }
}

MultiSpeedDialWidget::~MultiSpeedDialWidget()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void MultiSpeedDialWidget::slotFadeInChanged(int idx)
{
    emit fadeInChanged(idx, m_fadeIn[idx]->value());
}

void MultiSpeedDialWidget::slotFadeOutChanged(int idx)
{
    emit fadeOutChanged(idx, m_fadeOut[idx]->value());
}

void MultiSpeedDialWidget::slotHoldChanged(int idx)
{
    emit holdChanged(idx, m_hold[idx]->value());
}

/****************************************************************************
 * Speed settings
 ****************************************************************************/

void MultiSpeedDialWidget::setFadeInTitle(quint32 idx, const QString& title)
{
    m_fadeIn[idx]->setTitle(title);
}

void MultiSpeedDialWidget::setFadeInEnabled(quint32 idx, bool enable)
{
    m_fadeIn[idx]->setEnabled(enable);
}

void MultiSpeedDialWidget::setFadeInVisible(quint32 idx, bool set)
{
    m_fadeIn[idx]->setVisible(set);
}

void MultiSpeedDialWidget::setFadeIn(quint32 idx, int ms)
{
    m_fadeIn[idx]->setValue(ms);
}

int MultiSpeedDialWidget::fadeIn(quint32 idx) const
{
    return m_fadeIn[idx]->value();
}

void MultiSpeedDialWidget::setFadeOutTitle(quint32 idx, const QString& title)
{
    m_fadeOut[idx]->setTitle(title);
}

void MultiSpeedDialWidget::setFadeOutEnabled(quint32 idx, bool enable)
{
    m_fadeOut[idx]->setEnabled(enable);
}

void MultiSpeedDialWidget::setFadeOutVisible(quint32 idx, bool set)
{
    m_fadeOut[idx]->setVisible(set);
}

void MultiSpeedDialWidget::setFadeOut(quint32 idx, int ms)
{
    m_fadeOut[idx]->setValue(ms);
}

int MultiSpeedDialWidget::fadeOut(quint32 idx) const
{
    return m_fadeOut[idx]->value();
}

void MultiSpeedDialWidget::setHoldTitle(quint32 idx, const QString& title)
{
    m_hold[idx]->setTitle(title);
}

void MultiSpeedDialWidget::setHoldEnabled(quint32 idx, bool enable)
{
    m_hold[idx]->setEnabled(enable);
}

void MultiSpeedDialWidget::setHoldVisible(quint32 idx, bool set)
{
    m_hold[idx]->setVisible(set);
}

void MultiSpeedDialWidget::setHold(quint32 idx, int ms)
{
    m_hold[idx]->setValue(ms);
}

int MultiSpeedDialWidget::hold(quint32 idx) const
{
    return m_hold[idx]->value();
}

/************************************************************************
 * Optional text
 ************************************************************************/

void MultiSpeedDialWidget::setOptionalTextTitle(const QString& title)
{
    m_optionalTextGroup->setTitle(title);
    m_optionalTextGroup->setVisible(!title.isEmpty());
}

QString MultiSpeedDialWidget::optionalTextTitle() const
{
    return m_optionalTextGroup->title();
}

void MultiSpeedDialWidget::setOptionalText(const QString& text)
{
    m_optionalTextEdit->setText(text);
}

QString MultiSpeedDialWidget::optionalText() const
{
    return m_optionalTextEdit->text();
}

