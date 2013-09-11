/*
  Q Light Controller
  functionselection_test.h

  Copyright (C) Jano Svitok

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

#ifndef FUNCTIONSELECTION_TEST_H
#define FUNCTIONSELECTION_TEST_H

#include <QObject>
#include <QVariant>

class Doc;
class FunctionSelection_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void initial();
    void rememberFilter();
    void constFilter();

private:
    Doc* m_doc;
    QVariant m_savedFilter;
};

#endif
