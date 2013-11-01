/*
  Q Light Controller
  rgbalgorithm.h

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

#ifndef RGBALGORITHM_H
#define RGBALGORITHM_H

#include <QString>
#include <QVector>
#include <QSize>

class QDomDocument;
class QDomElement;

typedef QVector<QVector<uint> > RGBMap;

#define KXMLQLCRGBAlgorithm "Algorithm"
#define KXMLQLCRGBAlgorithmType "Type"

class RGBAlgorithm
{
public:
    virtual ~RGBAlgorithm() { /* NOP */ }

    enum Type
    {
        Text,
        Script,
        Image
    };

    /** Create a clone of the algorithm. Caller takes ownership of the pointer. */
    virtual RGBAlgorithm* clone() const = 0;

    /************************************************************************
     * RGB API
     ************************************************************************/
public:
    /** Maximum step count for rgbMap() function. */
    virtual int rgbMapStepCount(const QSize& size) = 0;

    /** Get the RGBMap for the given step. */
    virtual RGBMap rgbMap(const QSize& size, uint rgb, int step) = 0;

    /** Get the name of the algorithm. */
    virtual QString name() const = 0;

    /** Get the algorithm's author's name. */
    virtual QString author() const = 0;

    /** Get the algorithm's API version. 0 == invalid or unevaluated script. */
    virtual int apiVersion() const = 0;

    /** Get the algorithm's type */
    virtual Type type() const = 0;

    /************************************************************************
     * Available algorithms
     ************************************************************************/
public:
    static QStringList algorithms();
    static RGBAlgorithm* algorithm(const QString& name);

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** Load an RGBAlgorithm from a workspace file and return it as a new pointer. */
    static RGBAlgorithm* loader(const QDomElement& root);

    /** Save the contents of an RGBAlgorithm (run-time info) to a workspace file. */
    virtual bool saveXML(QDomDocument* doc, QDomElement* root) const = 0;
};

#endif
