/*
  Q Light Controller
  vcframeproperties.h

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

#ifndef VCFRAMEPROPERTIES_H
#define VCFRAMEPROPERTIES_H

#include <QDialog>
#include <QComboBox>
#include "ui_vcframeproperties.h"
#include "vcframepageshortcut.h"

class InputSelectionWidget;
class VCFrame;
class Doc;

/** @addtogroup ui_vc_props
 * @{
 */

class VCFrameProperties : public QDialog, public Ui_VCFrameProperties
{
    Q_OBJECT
    Q_DISABLE_COPY(VCFrameProperties)

public:
    VCFrameProperties(QWidget* parent, VCFrame *frame, Doc *doc);
    ~VCFrameProperties();

    bool allowChildren() const;
    bool allowResize() const;
    bool showHeader() const;
    QString frameName() const;
    bool multipageEnabled() const;
    bool cloneWidgets() const;
    bool pagesLoop() const;

protected slots:
    void slotMultipageChecked(bool enable);
    void slotPageComboChanged(int index);
    void slotTotalPagesNumberChanged(int number);
    void slotPageNameEditingFinished();

    void slotInputValueChanged(quint32 universe, quint32 channel);
    void slotKeySequenceChanged(QKeySequence key);

protected:
    VCFrame *m_frame;
    Doc* m_doc;
    InputSelectionWidget *m_inputEnableWidget;
    InputSelectionWidget *m_inputNextPageWidget;
    InputSelectionWidget *m_inputPrevPageWidget;
    QList<VCFramePageShortcut*> m_shortcuts;
    InputSelectionWidget *m_shortcutInputWidget;

public slots:
    void accept();
};

/** @} */

#endif
