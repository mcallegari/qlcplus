/*
  Q Light Controller
  vcbuttonproperties.h

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

#ifndef VCBUTTONPROPERTIES_H
#define VCBUTTONPROPERTIES_H

#include <QKeySequence>
#include <QDialog>

#include "ui_vcbuttonproperties.h"
#include "qlcinputsource.h"
#include "vcbutton.h"
#include "function.h"

class FunctionManager;
class KeyBind;

class VCButtonProperties : public QDialog, public Ui_VCButtonProperties
{
    Q_OBJECT
    Q_DISABLE_COPY(VCButtonProperties)

public:
    VCButtonProperties(VCButton* button, Doc* doc);
    ~VCButtonProperties();

protected slots:
    void slotAttachFunction();
    void slotSetFunction(quint32 fid = Function::invalidId());

    void slotAttachKey();
    void slotDetachKey();

    void slotAutoDetectInputToggled(bool checked);
    void slotInputValueChanged(quint32 universe, quint32 channel);
    void slotChooseInputClicked();

    void slotActionToggled();

    void slotIntensitySliderMoved(int value);
    void slotIntensityEdited(const QString& text);

    void accept();

protected:
    void updateInputSource();

protected:
    VCButton* m_button;
    Doc* m_doc;

    QKeySequence m_keySequence;
    quint32 m_function;
    QLCInputSource m_inputSource;
};

#endif
