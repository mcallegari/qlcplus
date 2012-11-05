/*
  Q Light Controller
  rgbalgorithm.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QStringList>
#include <QDebug>

#include "rgbalgorithm.h"
#include "rgbscript.h"
#include "rgbtext.h"

/****************************************************************************
 * Available algorithms
 ****************************************************************************/

QStringList RGBAlgorithm::algorithms()
{
    QStringList list;
    RGBText text;
    list << text.name();
    list << RGBScript::scriptNames();
    return list;
}

RGBAlgorithm* RGBAlgorithm::algorithm(const QString& name)
{
    RGBText text;
    if (name == text.name())
        return text.clone();
    else
        return RGBScript::script(name).clone();
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

RGBAlgorithm* RGBAlgorithm::loader(const QDomElement& root)
{
    RGBAlgorithm* algo = NULL;

    if (root.tagName() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return NULL;
    }

    QString type = root.attribute(KXMLQLCRGBAlgorithmType);
    if (type == KXMLQLCRGBText)
    {
        RGBText text;
        if (text.loadXML(root) == true)
            algo = text.clone();
    }
    else if (type == KXMLQLCRGBScript)
    {
        RGBScript scr = RGBScript::script(root.text());
        if (scr.apiVersion() > 0 && scr.name().isEmpty() == false)
            algo = scr.clone();
    }
    else
    {
        qWarning() << "Unrecognized RGB algorithm type:" << type;
    }

    return algo;
}
