/*
  Q Light Controller
  vcxypadfixture_test.h

  Copyright (C) Heikki Junnila

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

#ifndef VCXYPADFIXTURE_TEST_H
#define VCXYPADFIXTURE_TEST_H

#include <QObject>

class Doc;
class VCXYPadFixture_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void initial();
    void params();
    void fromVariantBelowZero();
    void fromVariantAboveOne();
    void fromVariantWithinRange();
    void fromVariantWrongSize();
    void fromVariantWrongVariant();
    void toVariant();
    void copy();
    void compare();
    void name();

    void loadXMLWrongRoot();
    void loadXMLHappy();
    void loadXMLSad();
    void saveXMLHappy();
    void saveXMLSad();

    void armNoFixture();
    void armDimmer();
    void arm8bit();
    void arm16bit();
    void disarm();

    void writeDimmer();
    void write8bitNoReverse();
    void write8bitReverse();
    void write16bitNoReverse();
    void write16bitReverse();

    void cleanupTestCase();

private:
    Doc* m_doc;
};

#endif
