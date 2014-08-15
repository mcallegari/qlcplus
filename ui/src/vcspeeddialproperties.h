/*
  Q Light Controller
  vcspeeddialproperties.h

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

#ifndef VCSPEEDDIALPROPERTIES_H
#define VCSPEEDDIALPROPERTIES_H

#include <QDialog>
#include "ui_vcspeeddialproperties.h"
#include "qlcinputsource.h"

class VCSpeedDial;
class VCSpeedDialFunction;
class Doc;

/** @addtogroup ui_vc_props
 * @{
 */

class VCSpeedDialProperties : public QDialog, public Ui_VCSpeedDialProperties
{
    Q_OBJECT

public:
    VCSpeedDialProperties(VCSpeedDial* dial, Doc* doc);
    ~VCSpeedDialProperties();

public slots:
    /** @reimp */
    void accept();

private:
    VCSpeedDial* m_dial;
    Doc* m_doc;

    /************************************************************************
     * Functions page
     ************************************************************************/
private slots:
    void slotAddClicked();
    void slotRemoveClicked();

private:
    /** Generate a QList of functions currently in the tree widget */
    QList <VCSpeedDialFunction> functions() const;

    /** Create a tree item for the given function $id */
    void createFunctionItem(const VCSpeedDialFunction &speeddialfunction);

    /************************************************************************
     * Input page
     ************************************************************************/
private:
    void updateInputSources();

private slots:
    void slotAutoDetectAbsoluteInputSourceToggled(bool checked);
    void slotChooseAbsoluteInputSourceClicked();
    void slotAbsoluteInputValueChanged(quint32 universe, quint32 channel);

    void slotAutoDetectTapInputSourceToggled(bool checked);
    void slotChooseTapInputSourceClicked();
    void slotTapInputValueChanged(quint32 universe, quint32 channel);

    void slotAttachKey();
    void slotDetachKey();

private:
    QLCInputSource *m_absoluteInputSource;
    QLCInputSource *m_tapInputSource;
    QKeySequence m_tapKeySequence;
};

/** @} */

#endif
