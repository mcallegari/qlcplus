/*
  Q Light Controller
  vcbutton_test.h

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

#ifndef VCBUTTON_TEST_H
#define VCBUTTON_TEST_H

#include <QObject>

class Doc;
class VCButton_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void init();
    void cleanup();

    void initial();
    void function();
    void action();
    void intensity();
    void bgcolor();
    void fgcolor();
    void resetColors();
    void iconPath();
    void on();
    void keySequence();
    void copy();
    void load();
    void save();
    void customMenu();
    void toggle();
    void flash();
    void input();
    void paint();

    // https://github.com/mcallegari/qlcplus/issues/116
    void toggleAndFlash();

private:
    Doc* m_doc;
};

#endif
