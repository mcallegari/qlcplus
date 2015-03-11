/*
  Q Light Controller
  vcbuttonproperties.h

  Copyright (c) Heikki Junnila

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
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
class SpeedDialWidget;
class KeyBind;

/** @addtogroup ui_vc_props
 * @{
 */

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
    QLCInputSource *m_inputSource;

    /************************************************************************
     * Speed dial
     ************************************************************************/

private slots:
    void slotSpeedDialToggle(bool state);
    void slotFadeOutDialChanged(int ms);
    void slotDialDestroyed(QObject* dial);

private:
    SpeedDialWidget *m_speedDials;
    int m_fadeOutTime;

};

/** @} */

#endif
