/*
  Q Light Controller
  vcframeproperties.cpp

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
    if (frame->multipageMode() == true)
        m_showHeaderCheck->setEnabled(false);
    m_totalPagesSpin->setValue(frame->totalPagesNumber());
    if (frame->totalPagesNumber() != 1)
        m_cloneFirstPageCheck->setEnabled(false);

    connect(m_enablePaging, SIGNAL(toggled(bool)),
            this, SLOT(slotMultipageChecked(bool)));

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

void VCFrameProperties::slotMultipageChecked(bool enable)
{
    if (enable == true)
    {
        m_showHeaderCheck->setChecked(true);
        m_showHeaderCheck->setEnabled(false);
    }
}

void VCFrameProperties::accept()
{
    bool hasHeader = m_frame->isHeaderVisible();

    m_frame->setCaption(m_frameName->text());
    m_frame->setAllowChildren(m_allowChildrenCheck->isChecked());
    m_frame->setAllowResize(m_allowResizeCheck->isChecked());

    /* If the frame is coming from a headerless state,
     * all the children widgets must be moved down */
    if (m_showHeaderCheck->isChecked() && hasHeader == false)
    {
        QListIterator <VCWidget*> it(m_frame->findChildren<VCWidget*>());

        // resize the frame too if it contains children
        if (it.hasNext())
            m_frame->resize(QSize(m_frame->width(), m_frame->height() + 40));

        while (it.hasNext() == true)
        {
            VCWidget* child = it.next();

            // move only first level children
            if (child->parentWidget() == m_frame)
                child->move(QPoint(child->x(), child->y() + 40));
        }
    }
    m_frame->setHeaderVisible(m_showHeaderCheck->isChecked());
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
