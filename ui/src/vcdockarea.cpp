/*
  Q Light Controller
  vcdockarea.cpp

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

#include <QVBoxLayout>
#include <QString>
#include <QDebug>
#include <QtXml>

#include "qlcfile.h"

#include "grandmasterslider.h"
#include "virtualconsole.h"
#include "vcproperties.h"
#include "vcdockarea.h"
#include "outputmap.h"
#include "inputmap.h"

VCDockArea::VCDockArea(QWidget* parent, OutputMap* outputMap, InputMap* inputMap)
    : QFrame(parent)
{
    Q_ASSERT(outputMap != NULL);
    Q_ASSERT(inputMap != NULL);

    new QHBoxLayout(this);
    layout()->setMargin(0);
    layout()->setSpacing(1);

    m_gm = new GrandMasterSlider(this, outputMap, inputMap);
    layout()->addWidget(m_gm);
}

VCDockArea::~VCDockArea()
{
}

void VCDockArea::setGrandMasterInvertedAppearance(UniverseArray::GMSliderMode mode)
{
    Q_ASSERT(m_gm != NULL);
    if (mode == UniverseArray::GMNormal)
        m_gm->setInvertedAppearance(false);
    else
        m_gm->setInvertedAppearance(true);
}

/*****************************************************************************
 * Event Handlers
 *****************************************************************************/

void VCDockArea::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    emit visibilityChanged(true);
}

void VCDockArea::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event);
    emit visibilityChanged(false);
}
