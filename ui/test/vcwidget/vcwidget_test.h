/*
  Q Light Controller
  vcwidget_test.h

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

#ifndef VCWIDGET_TEST_H
#define VCWIDGET_TEST_H

#include <QObject>

class Doc;
class VCWidget_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void initial();
    void bgImage();
    void bgColor();
    void fgColor();
    void resetBg();
    void resetFg();
    void font();
    void caption();
    void frame();
    void inputSource();
    void copy();
    void stripKeySequence();
    void keyPress();
    void loadInput();
    void loadAppearance();
    void saveInput();
    void saveAppearance();
    void saveAppearanceDefaultsImage();
    void saveWindowState();
    void loadWindowState();
    void resize();
    void move();
    void paint();
    void mousePress();

private:
    Doc* m_doc;
};

#endif
