/*
  Q Light Controller
  vcbuttonproperties.cpp

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

#include <QIntValidator>
#include <QKeySequence>
#include <QRadioButton>
#include <QMessageBox>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <qmath.h>

#include "qlcinputchannel.h"
#include "qlcinputprofile.h"
#include "qlcfixturedef.h"

#include "vcbuttonproperties.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "speeddialwidget.h"
#include "virtualconsole.h"
#include "assignhotkey.h"
#include "inputpatch.h"
#include "function.h"
#include "fixture.h"
#include "doc.h"

VCButtonProperties::VCButtonProperties(VCButton* button, Doc* doc)
    : QDialog(button)
    , m_doc(doc)
    , m_speedDials(NULL)
{
    Q_ASSERT(button != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    /* Button text and function */
    m_button = button;
    m_nameEdit->setText(m_button->caption());
    slotSetFunction(button->function());

    /* Key sequence */
    m_keySequence = QKeySequence(button->keySequence());
    m_keyEdit->setText(m_keySequence.toString(QKeySequence::NativeText));

    /* External input */
    m_inputSource = m_button->inputSource();
    updateInputSource();

    /* Press action */
    if (button->action() == VCButton::Flash)
        m_flash->setChecked(true);
    else if (button->action() == VCButton::Blackout)
        m_blackout->setChecked(true);
    else if (button->action() == VCButton::StopAll)
        m_stopAll->setChecked(true);
    else
        m_toggle->setChecked(true);
    m_fadeOutTime = m_button->stopAllFadeTime();
    m_fadeOutEdit->setText(Function::speedToString(m_fadeOutTime));
    slotActionToggled();

    /* Intensity adjustment */
    m_intensityEdit->setValidator(new QIntValidator(0, 100, this));
    m_intensityGroup->setChecked(m_button->isStartupIntensityEnabled());
    int intensity = int(floor(m_button->startupIntensity() * double(100)));
    m_intensityEdit->setText(QString::number(intensity));
    m_intensitySlider->setValue(intensity);

    /* Button connections */
    connect(m_attachFunction, SIGNAL(clicked()), this, SLOT(slotAttachFunction()));
    connect(m_detachFunction, SIGNAL(clicked()), this, SLOT(slotSetFunction()));

    connect(m_attachKey, SIGNAL(clicked()), this, SLOT(slotAttachKey()));
    connect(m_detachKey, SIGNAL(clicked()), this, SLOT(slotDetachKey()));

    connect(m_toggle, SIGNAL(toggled(bool)), this, SLOT(slotActionToggled()));
    connect(m_blackout, SIGNAL(toggled(bool)), this, SLOT(slotActionToggled()));
    connect(m_stopAll, SIGNAL(toggled(bool)), this, SLOT(slotActionToggled()));
    connect(m_flash, SIGNAL(toggled(bool)), this, SLOT(slotActionToggled()));

    connect(m_speedDialButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSpeedDialToggle(bool)));

    connect(m_autoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectInputToggled(bool)));
    connect(m_chooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotChooseInputClicked()));

    connect(m_intensitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotIntensitySliderMoved(int)));
    connect(m_intensityEdit, SIGNAL(textEdited(QString)),
            this, SLOT(slotIntensityEdited(QString)));
}

VCButtonProperties::~VCButtonProperties()
{
}

void VCButtonProperties::slotAttachFunction()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(false);
    if (fs.exec() == QDialog::Accepted && fs.selection().size() > 0)
        slotSetFunction(fs.selection().first());
}

void VCButtonProperties::slotSetFunction(quint32 fid)
{
    m_function = fid;
    Function* func = m_doc->function(m_function);
    m_attributesList->clear();
    if (func == NULL)
    {
        m_functionEdit->setText(tr("No function"));
    }
    else
    {
        m_functionEdit->setText(func->name());
        if (m_nameEdit->text().simplified().contains(QString::number(m_button->id())))
            m_nameEdit->setText(func->name());

        foreach(Attribute attr, func->attributes())
        {
            QListWidgetItem *item = new QListWidgetItem(attr.name);
            //item->setCheckState(Qt::Checked);
            m_attributesList->addItem(item);
        }
    }
}

void VCButtonProperties::slotAttachKey()
{
    AssignHotKey ahk(this, m_keySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_keySequence = QKeySequence(ahk.keySequence());
        m_keyEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
    }
}

void VCButtonProperties::slotDetachKey()
{
    m_keySequence = QKeySequence();
    m_keyEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
}

void VCButtonProperties::slotAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputOutputMap(),
                SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(),
                   SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
}

void VCButtonProperties::slotInputValueChanged(quint32 universe, quint32 channel)
{
    m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(universe, (m_button->page() << 16) | channel));
    updateInputSource();
}

void VCButtonProperties::slotChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputOutputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(sic.universe(), sic.channel()));
        updateInputSource();
    }
}

void VCButtonProperties::updateInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputOutputMap()->inputSourceNames(m_inputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }

    m_inputUniverseEdit->setText(uniName);
    m_inputChannelEdit->setText(chName);
}

void VCButtonProperties::slotActionToggled()
{
    if (m_blackout->isChecked() == true || m_stopAll->isChecked() == true)
    {
        m_generalGroup->setEnabled(false);
        m_intensityGroup->setEnabled(false);
    }
    else
    {
        m_generalGroup->setEnabled(true);
        m_intensityGroup->setEnabled(true);
    }

    m_fadeOutEdit->setEnabled(m_stopAll->isChecked());
    m_safFadeLabel->setEnabled(m_stopAll->isChecked());
    m_speedDialButton->setEnabled(m_stopAll->isChecked());
}

void VCButtonProperties::slotSpeedDialToggle(bool state)
{
    if (state == true)
    {
        m_speedDials = new SpeedDialWidget(this);
        m_speedDials->setAttribute(Qt::WA_DeleteOnClose);
        m_speedDials->setWindowTitle(m_button->caption());
        m_speedDials->setFadeInVisible(false);
        m_speedDials->setFadeOutSpeed(m_button->stopAllFadeTime());
        m_speedDials->setDurationEnabled(false);
        m_speedDials->setDurationVisible(false);
        connect(m_speedDials, SIGNAL(fadeOutChanged(int)), this, SLOT(slotFadeOutDialChanged(int)));
        connect(m_speedDials, SIGNAL(destroyed(QObject*)), this, SLOT(slotDialDestroyed(QObject*)));
        m_speedDials->show();
    }
    else
    {
        if (m_speedDials != NULL)
            m_speedDials->deleteLater();
        m_speedDials = NULL;
    }
}

void VCButtonProperties::slotFadeOutDialChanged(int ms)
{
    m_fadeOutEdit->setText(Function::speedToString(ms));
    m_fadeOutTime = ms;
}

void VCButtonProperties::slotDialDestroyed(QObject *)
{
    m_speedDialButton->setChecked(false);
}

void VCButtonProperties::slotIntensitySliderMoved(int value)
{
    m_intensityEdit->setText(QString::number(value));
}

void VCButtonProperties::slotIntensityEdited(const QString& text)
{
    m_intensitySlider->setValue(text.toInt());
}

void VCButtonProperties::accept()
{
    m_button->setCaption(m_nameEdit->text());
    m_button->setFunction(m_function);
    m_button->setKeySequence(m_keySequence);
    m_button->setInputSource(m_inputSource);
    m_button->enableStartupIntensity(m_intensityGroup->isChecked());
    m_button->setStartupIntensity(qreal(m_intensitySlider->value()) / qreal(100));

    if (m_toggle->isChecked() == true)
        m_button->setAction(VCButton::Toggle);
    else if (m_blackout->isChecked() == true)
        m_button->setAction(VCButton::Blackout);
    else if (m_stopAll->isChecked() == true)
    {
        m_button->setAction(VCButton::StopAll);
        m_button->setStopAllFadeOutTime(m_fadeOutTime);
    }
    else
        m_button->setAction(VCButton::Flash);

    m_button->updateOnState();

    QDialog::accept();
}

