/*
  Q Light Controller
  vcspeeddialproperties.cpp

  Copyright (c) Heikki Junnila

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

#include "vcspeeddialproperties.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "assignhotkey.h"
#include "vcspeeddial.h"
#include "doc.h"

#define COL_NAME 0
#define COL_TYPE 1
#define PROP_ID  Qt::UserRole

VCSpeedDialProperties::VCSpeedDialProperties(VCSpeedDial* dial, Doc* doc)
    : QDialog(dial)
    , m_dial(dial)
    , m_doc(doc)
{
    Q_ASSERT(dial != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    /* Name */
    m_nameEdit->setText(m_dial->caption());

    /* Functions */
    foreach (quint32 id, m_dial->functions())
        createFunctionItem(id);

    /* Speed types */
    if (dial->speedTypes() & VCSpeedDial::FadeIn)
        m_fadeInCheck->setChecked(true);
    if (dial->speedTypes() & VCSpeedDial::FadeOut)
        m_fadeOutCheck->setChecked(true);
    if (dial->speedTypes() & VCSpeedDial::Duration)
        m_durationCheck->setChecked(true);

    /* Absolute input */
    m_absoluteMinSpin->setValue(m_dial->absoluteValueMin() / 1000);
    m_absoluteMaxSpin->setValue(m_dial->absoluteValueMax() / 1000);
    m_absoluteInputSource = m_dial->inputSource(VCSpeedDial::absoluteInputSourceId);

    /* Tap input */
    m_tapInputSource = m_dial->inputSource(VCSpeedDial::tapInputSourceId);

    /* Key sequence */
    m_tapKeySequence = QKeySequence(dial->keySequence());
    m_keyEdit->setText(m_tapKeySequence.toString(QKeySequence::NativeText));

    updateInputSources();

    connect(m_autoDetectAbsoluteInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectAbsoluteInputSourceToggled(bool)));
    connect(m_autoDetectTapInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectTapInputSourceToggled(bool)));
    connect(m_attachKey, SIGNAL(clicked()), this, SLOT(slotAttachKey()));
    connect(m_detachKey, SIGNAL(clicked()), this, SLOT(slotDetachKey()));
}

VCSpeedDialProperties::~VCSpeedDialProperties()
{
}

void VCSpeedDialProperties::accept()
{
    /* Name */
    m_dial->setCaption(m_nameEdit->text());

    /* Functions */
    m_dial->setFunctions(functions());

    /* Speed types */
    VCSpeedDial::SpeedTypes types = 0;
    if (m_fadeInCheck->isChecked() == true)
        types |= VCSpeedDial::FadeIn;
    if (m_fadeOutCheck->isChecked() == true)
        types |= VCSpeedDial::FadeOut;
    if (m_durationCheck->isChecked() == true)
        types |= VCSpeedDial::Duration;
    m_dial->setSpeedTypes(types);

    /* Input sources */
    m_dial->setAbsoluteValueRange(m_absoluteMinSpin->value() * 1000,
                                  m_absoluteMaxSpin->value() * 1000);
    m_dial->setInputSource(m_absoluteInputSource, VCSpeedDial::absoluteInputSourceId);
    m_dial->setInputSource(m_tapInputSource, VCSpeedDial::tapInputSourceId);

    m_dial->setKeySequence(m_tapKeySequence);

    QDialog::accept();
}

/****************************************************************************
 * Functions page
 ****************************************************************************/

void VCSpeedDialProperties::slotAddClicked()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setDisabledFunctions(functions().toList());
    if (fs.exec() == QDialog::Accepted)
    {
        foreach (quint32 id, fs.selection())
            createFunctionItem(id);
    }
}

void VCSpeedDialProperties::slotRemoveClicked()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
        delete it.next();
}

QSet <quint32> VCSpeedDialProperties::functions() const
{
    QSet <quint32> set;
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);

        QVariant var = item->data(COL_NAME, PROP_ID);
        if (var.isValid() == true)
            set << var.toUInt();
    }

    return set;
}

void VCSpeedDialProperties::createFunctionItem(quint32 id)
{
    Function* function = m_doc->function(id);
    if (function != NULL)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(COL_NAME, function->name());
        item->setText(COL_TYPE, function->typeString());
        item->setData(COL_NAME, PROP_ID, id);
    }
}

/****************************************************************************
 * Input page
 ****************************************************************************/

void VCSpeedDialProperties::updateInputSources()
{
    QString uniName;
    QString chName;

    // Absolute
    if (m_doc->inputMap()->inputSourceNames(m_absoluteInputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }
    m_absoluteInputUniverseEdit->setText(uniName);
    m_absoluteInputChannelEdit->setText(chName);

    // Tap
    if (m_doc->inputMap()->inputSourceNames(m_tapInputSource, uniName, chName) == false)
    {
        uniName = KInputNone;
        chName = KInputNone;
    }
    m_tapInputUniverseEdit->setText(uniName);
    m_tapInputChannelEdit->setText(chName);
}

void VCSpeedDialProperties::slotAbsoluteInputValueChanged(quint32 universe, quint32 channel)
{
    qDebug() << "Signal received !";
    m_absoluteInputSource = QLCInputSource(universe, channel);
    updateInputSources();
}

void VCSpeedDialProperties::slotTapInputValueChanged(quint32 universe, quint32 channel)
{
    m_tapInputSource = QLCInputSource(universe, channel);
    updateInputSources();
}

void VCSpeedDialProperties::slotAutoDetectAbsoluteInputSourceToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotAbsoluteInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotAbsoluteInputValueChanged(quint32,quint32)));
    }
}

void VCSpeedDialProperties::slotChooseAbsoluteInputSourceClicked()
{
    SelectInputChannel sic(this, m_doc->inputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_absoluteInputSource = QLCInputSource(sic.universe(), sic.channel());
        updateInputSources();
    }
}

void VCSpeedDialProperties::slotAutoDetectTapInputSourceToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotTapInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotTapInputValueChanged(quint32,quint32)));
    }
}

void VCSpeedDialProperties::slotChooseTapInputSourceClicked()
{
    SelectInputChannel sic(this, m_doc->inputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_tapInputSource = QLCInputSource(sic.universe(), sic.channel());
        updateInputSources();
    }
}

void VCSpeedDialProperties::slotAttachKey()
{
    AssignHotKey ahk(this, m_tapKeySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_tapKeySequence = QKeySequence(ahk.keySequence());
        m_keyEdit->setText(m_tapKeySequence.toString(QKeySequence::NativeText));
    }
}

void VCSpeedDialProperties::slotDetachKey()
{
    m_tapKeySequence = QKeySequence();
    m_keyEdit->setText(m_tapKeySequence.toString(QKeySequence::NativeText));
}
