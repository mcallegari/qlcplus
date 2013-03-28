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

#include "vcframeproperties.h"
#include "vcframe.h"

VCFrameProperties::VCFrameProperties(QWidget* parent, const VCFrame* frame)
    : QDialog(parent)
{
    Q_ASSERT(frame != NULL);
    setupUi(this);

    m_frameName->setText(frame->caption());
    m_allowChildrenCheck->setChecked(frame->allowChildren());
    m_allowResizeCheck->setChecked(frame->allowResize());
    m_showHeaderCheck->setChecked(frame->isHeaderVisible());
}

VCFrameProperties::~VCFrameProperties()
{
}

bool VCFrameProperties::allowChildren() const
{
    return m_allowChildren;
}

bool VCFrameProperties::allowResize() const
{
    return m_allowResize;
}

bool VCFrameProperties::showHeader() const
{
    return m_showHeader;
}

QString VCFrameProperties::frameName() const
{
    return m_frameName->text();
}

void VCFrameProperties::accept()
{
    m_allowChildren = m_allowChildrenCheck->isChecked();
    m_allowResize = m_allowResizeCheck->isChecked();
    m_showHeader = m_showHeaderCheck->isChecked();
    QDialog::accept();
}
