/*
  Q Light Controller Plus
  qlcclipboard.cpp

  Copyright (c) Massimo Callegari

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

#include "qlcclipboard.h"
#include "chaser.h"
#include "scene.h"

QLCClipboard::QLCClipboard(Doc *doc)
    : m_doc(doc)
    , m_copyFunction(NULL)
{

}

void QLCClipboard::resetContents()
{
    m_copySteps.clear();
    if (m_copyFunction != NULL && m_doc->function(m_copyFunction->id()) == NULL)
        delete m_copyFunction;
    m_copyFunction = NULL;
}

void QLCClipboard::copyContent(quint32 sourceID, QList<ChaserStep> steps)
{
    Q_UNUSED(sourceID)

    m_copySteps.clear();
    m_copySteps = steps;
}

void QLCClipboard::copyContent(quint32 sourceID, Function *function)
{
    Q_UNUSED(sourceID)

    if (function == NULL)
        return;

    if (m_copyFunction != NULL && m_doc->function(m_copyFunction->id()) == NULL)
        delete m_copyFunction;
    m_copyFunction = NULL;

    /* Attempt to create a copy of the function to Doc */
    Function* copy = function->createCopy(m_doc, false);
    if (copy != NULL)
    {
        copy->setName(tr("Copy of %1").arg(function->name()));
        m_copyFunction = copy;
    }
}

bool QLCClipboard::hasChaserSteps()
{
    if (m_copySteps.count() > 0)
        return true;

    return false;
}

bool QLCClipboard::hasFunction()
{
    if (m_copyFunction != NULL)
        return true;

    return false;
}

QList<ChaserStep> QLCClipboard::getChaserSteps()
{
    return m_copySteps;
}

Function *QLCClipboard::getFunction()
{
    return m_copyFunction;
}








