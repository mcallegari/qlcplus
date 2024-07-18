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

#include "speeddialwidget.h"
#include "speeddial.h"
#include "apputil.h"

#define WINDOW_FLAGS Qt::WindowFlags( \
    (Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Window | \
     Qt::WindowStaysOnTopHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint))

#define SETTINGS_GEOMETRY "speeddialwidget/geometry"
#define SETTINGS_DIRECTION "speeddialwidget/direction"

SpeedDialWidget::SpeedDialWidget(QWidget* parent)
    : QWidget(parent)
    , m_fadeIn(NULL)
    , m_fadeOut(NULL)
    , m_hold(NULL)
    , m_optionalTextGroup(NULL)
    , m_optionalTextEdit(NULL)
{
    QSettings settings;
    QVariant var;
    QBoxLayout* lay = NULL;

    setWindowFlags(WINDOW_FLAGS);

    /* Layout with customizable direction */
    var = settings.value(SETTINGS_DIRECTION);
    if (var.isValid() == true)
        lay = new QBoxLayout(QBoxLayout::Direction(var.toInt()), this);
    else
        lay = new QBoxLayout(QBoxLayout::TopToBottom, this);

    /* Create dials */
    m_fadeIn = new SpeedDial(this);
    m_fadeIn->setTitle(tr("Fade In"));
    layout()->addWidget(m_fadeIn);
    connect(m_fadeIn, SIGNAL(valueChanged(int)), this, SIGNAL(fadeInChanged(int)));
    connect(m_fadeIn, SIGNAL(tapped()), this, SIGNAL(fadeInTapped()));

    m_fadeOut = new SpeedDial(this);
    m_fadeOut->setTitle(tr("Fade Out"));
    layout()->addWidget(m_fadeOut);
    connect(m_fadeOut, SIGNAL(valueChanged(int)), this, SIGNAL(fadeOutChanged(int)));
    connect(m_fadeOut, SIGNAL(tapped()), this, SIGNAL(fadeOutTapped()));

    m_hold = new SpeedDial(this);
    m_hold->setTitle(tr("Hold"));
    layout()->addWidget(m_hold);
    connect(m_hold, SIGNAL(valueChanged(int)), this, SIGNAL(holdChanged(int)));
    connect(m_hold, SIGNAL(tapped()), this, SIGNAL(holdTapped()));

    /* Optional text */
    m_optionalTextGroup = new QGroupBox(this);
    layout()->addWidget(m_optionalTextGroup);
    new QVBoxLayout(m_optionalTextGroup);
    m_optionalTextEdit = new QLineEdit(m_optionalTextGroup);
    m_optionalTextGroup->layout()->addWidget(m_optionalTextEdit);
    m_optionalTextGroup->setVisible(false);
    connect(m_optionalTextEdit, SIGNAL(textEdited(const QString&)),
            this, SIGNAL(optionalTextEdited(const QString&)));

    lay->addStretch();

    /* Position */
    var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        this->restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

SpeedDialWidget::~SpeedDialWidget()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

/****************************************************************************
 * Speed settings
 ****************************************************************************/

void SpeedDialWidget::setFadeInTitle(const QString& title)
{
    m_fadeIn->setTitle(title);
}

void SpeedDialWidget::setFadeInEnabled(bool enable)
{
    m_fadeIn->setEnabled(enable);
}

void SpeedDialWidget::setFadeInVisible(bool set)
{
    m_fadeIn->setVisible(set);
}

void SpeedDialWidget::setFadeInSpeed(int ms)
{
    m_fadeIn->setValue(ms);
}

int SpeedDialWidget::fadeIn() const
{
    return m_fadeIn->value();
}

void SpeedDialWidget::setFadeOutTitle(const QString& title)
{
    m_fadeOut->setTitle(title);
}

void SpeedDialWidget::setFadeOutEnabled(bool enable)
{
    m_fadeOut->setEnabled(enable);
}

void SpeedDialWidget::setFadeOutVisible(bool set)
{
    m_fadeOut->setVisible(set);
}

void SpeedDialWidget::setFadeOutSpeed(int ms)
{
    m_fadeOut->setValue(ms);
}

int SpeedDialWidget::fadeOut() const
{
    return m_fadeOut->value();
}

void SpeedDialWidget::setDurationTitle(const QString& title)
{
    m_hold->setTitle(title);
}

void SpeedDialWidget::setDurationEnabled(bool enable)
{
    m_hold->setEnabled(enable);
}

void SpeedDialWidget::setDurationVisible(bool set)
{
    m_hold->setVisible(set);
}

void SpeedDialWidget::setDuration(int ms)
{
    m_hold->setValue(ms);
}

int SpeedDialWidget::duration() const
{
    return m_hold->value();
}

/************************************************************************
 * Optional text
 ************************************************************************/

void SpeedDialWidget::setOptionalTextTitle(const QString& title)
{
    m_optionalTextGroup->setTitle(title);
    m_optionalTextGroup->setVisible(!title.isEmpty());
}

QString SpeedDialWidget::optionalTextTitle() const
{
    return m_optionalTextGroup->title();
}

void SpeedDialWidget::setOptionalText(const QString& text)
{
    m_optionalTextEdit->setText(text);
}

QString SpeedDialWidget::optionalText() const
{
    return m_optionalTextEdit->text();
}
