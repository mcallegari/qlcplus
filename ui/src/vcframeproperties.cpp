/*
  Q Light Controller
  vcframeproperties.cpp

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

#include <QCheckBox>

#include "selectinputchannel.h"
#include "vcframeproperties.h"
#include "assignhotkey.h"
#include "vcframe.h"
#include "doc.h"

VCFrameProperties::VCFrameProperties(QWidget* parent, VCFrame* frame, Doc *doc)
    : QDialog(parent)
    , m_frame(frame)
    , m_doc(doc)
{
    Q_ASSERT(frame != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    m_frameName->setText(frame->caption());
    m_allowChildrenCheck->setChecked(frame->allowChildren());
    m_allowResizeCheck->setChecked(frame->allowResize());
    m_showHeaderCheck->setChecked(frame->isHeaderVisible());
    m_enablePaging->setChecked(frame->multipageMode());
    m_totalPagesSpin->setValue(frame->totalPagesNumber());
    if (frame->totalPagesNumber() != 1)
        m_cloneFirstPageCheck->setEnabled(false);

    /************************************************************************
     * Next page
     ************************************************************************/

    /* Connections */
    connect(m_nextAttachButton, SIGNAL(clicked()),
            this, SLOT(slotNextAttachClicked()));
    connect(m_nextDetachButton, SIGNAL(clicked()),
            this, SLOT(slotNextDetachClicked()));
    connect(m_nextAutoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotNextAutoDetectInputToggled(bool)));
    connect(m_nextChooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotNextChooseInputClicked()));

    /* Key binding */
    m_nextKeySequence = QKeySequence(frame->nextPageKeySequence());
    m_nextKeyEdit->setText(m_nextKeySequence.toString(QKeySequence::NativeText));

    /* External input */
    m_nextInputSource = frame->inputSource(VCFrame::nextPageInputSourceId);
    updateNextInputSource();

    /************************************************************************
     * Previous page
     ************************************************************************/

    /* Connections */
    connect(m_previousAttachButton, SIGNAL(clicked()),
            this, SLOT(slotPreviousAttachClicked()));
    connect(m_previousDetachButton, SIGNAL(clicked()),
            this, SLOT(slotPreviousDetachClicked()));
    connect(m_previousAutoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotPreviousAutoDetectInputToggled(bool)));
    connect(m_previousChooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotPreviousChooseInputClicked()));

    /* Key binding */
    m_previousKeySequence = QKeySequence(frame->previousPageKeySequence());
    m_previousKeyEdit->setText(m_previousKeySequence.toString(QKeySequence::NativeText));

    /* External input */
    m_previousInputSource = frame->inputSource(VCFrame::previousPageInputSourceId);
    updatePreviousInputSource();
}

VCFrameProperties::~VCFrameProperties()
{
}

bool VCFrameProperties::allowChildren() const
{
    return m_allowChildrenCheck->isChecked();
}

bool VCFrameProperties::allowResize() const
{
    return m_allowResizeCheck->isChecked();
}

bool VCFrameProperties::showHeader() const
{
    return m_showHeaderCheck->isChecked();
}

QString VCFrameProperties::frameName() const
{
    return m_frameName->text();
}

bool VCFrameProperties::multipageEnabled() const
{
    return m_enablePaging->isChecked();
}

bool VCFrameProperties::cloneWidgets() const
{
    return m_cloneFirstPageCheck->isChecked();
}

void VCFrameProperties::accept()
{
    m_frame->setCaption(m_frameName->text());
    m_frame->setAllowChildren(m_allowChildrenCheck->isChecked());
    m_frame->setAllowResize(m_allowResizeCheck->isChecked());
    m_frame->setShowHeader(m_showHeaderCheck->isChecked());
    m_frame->setMultipageMode(m_enablePaging->isChecked());
    m_frame->setTotalPagesNumber(m_totalPagesSpin->value());

    /* Key sequences */
    m_frame->setNextPageKeySequence(m_nextKeySequence);
    m_frame->setPreviousPageKeySequence(m_previousKeySequence);

    /* Input sources */
    m_frame->setInputSource(m_nextInputSource, VCFrame::nextPageInputSourceId);
    m_frame->setInputSource(m_previousInputSource, VCFrame::previousPageInputSourceId);

    QDialog::accept();
}

/****************************************************************************
 * Next page
 ****************************************************************************/

void VCFrameProperties::slotNextAttachClicked()
{
    AssignHotKey ahk(this, m_nextKeySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_nextKeySequence = QKeySequence(ahk.keySequence());
        m_nextKeyEdit->setText(m_nextKeySequence.toString(QKeySequence::NativeText));
    }
}

void VCFrameProperties::slotNextDetachClicked()
{
    m_nextKeySequence = QKeySequence();
    m_nextKeyEdit->setText(m_nextKeySequence.toString(QKeySequence::NativeText));
}

void VCFrameProperties::slotNextChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_nextInputSource = QLCInputSource(sic.universe(), sic.channel());
        updateNextInputSource();
    }
}

void VCFrameProperties::slotNextAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotNextInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotNextInputValueChanged(quint32,quint32)));
    }
}

void VCFrameProperties::slotNextInputValueChanged(quint32 uni, quint32 ch)
{
    m_nextInputSource = QLCInputSource(uni, ch);
    updateNextInputSource();
}

void VCFrameProperties::updateNextInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputMap()->inputSourceNames(m_nextInputSource, uniName, chName) == true)
    {
        /* Display the gathered information */
        m_nextInputUniverseEdit->setText(uniName);
        m_nextInputChannelEdit->setText(chName);
    }
    else
    {
        m_nextInputUniverseEdit->setText(KInputNone);
        m_nextInputChannelEdit->setText(KInputNone);
    }
}

/****************************************************************************
 * Previous page
 ****************************************************************************/

void VCFrameProperties::slotPreviousAttachClicked()
{
    AssignHotKey ahk(this, m_previousKeySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_previousKeySequence = QKeySequence(ahk.keySequence());
        m_previousKeyEdit->setText(m_previousKeySequence.toString(QKeySequence::NativeText));
    }
}

void VCFrameProperties::slotPreviousDetachClicked()
{
    m_previousKeySequence = QKeySequence();
    m_previousKeyEdit->setText(m_previousKeySequence.toString(QKeySequence::NativeText));
}

void VCFrameProperties::slotPreviousChooseInputClicked()
{
    SelectInputChannel sic(this, m_doc->inputMap());
    if (sic.exec() == QDialog::Accepted)
    {
        m_previousInputSource = QLCInputSource(sic.universe(), sic.channel());
        updatePreviousInputSource();
    }
}

void VCFrameProperties::slotPreviousAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotPreviousInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(m_doc->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotPreviousInputValueChanged(quint32,quint32)));
    }
}

void VCFrameProperties::slotPreviousInputValueChanged(quint32 uni, quint32 ch)
{
    m_previousInputSource = QLCInputSource(uni, ch);
    updatePreviousInputSource();
}

void VCFrameProperties::updatePreviousInputSource()
{
    QString uniName;
    QString chName;

    if (m_doc->inputMap()->inputSourceNames(m_previousInputSource, uniName, chName) == true)
    {
        /* Display the gathered information */
        m_previousInputUniverseEdit->setText(uniName);
        m_previousInputChannelEdit->setText(chName);
    }
    else
    {
        m_previousInputUniverseEdit->setText(KInputNone);
        m_previousInputChannelEdit->setText(KInputNone);
    }
}
