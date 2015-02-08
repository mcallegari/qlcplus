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
#include "ui_vcframeproperties.h"
#include "qlcinputsource.h"

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

protected:
    VCFrame *m_frame;
    Doc* m_doc;

    /************************************************************************
     * Enable control
     ************************************************************************/
protected slots:
    void slotEnableAttachClicked();
    void slotEnableDetachClicked();
    void slotEnableChooseInputClicked();
    void slotEnableAutoDetectInputToggled(bool checked);
    void slotEnableInputValueChanged(quint32 uni, quint32 ch);

protected:
    void updateEnableInputSource();

protected:
    QKeySequence m_enableKeySequence;
    QLCInputSource *m_enableInputSource;

    /************************************************************************
     * Next page
     ************************************************************************/
protected slots:
    void slotNextAttachClicked();
    void slotNextDetachClicked();
    void slotNextChooseInputClicked();
    void slotNextAutoDetectInputToggled(bool checked);
    void slotNextInputValueChanged(quint32 uni, quint32 ch);

protected:
    void updateNextInputSource();

protected:
    QKeySequence m_nextKeySequence;
    QLCInputSource *m_nextInputSource;

    /************************************************************************
     * Previous page
     ************************************************************************/
protected slots:
    void slotPreviousAttachClicked();
    void slotPreviousDetachClicked();
    void slotPreviousChooseInputClicked();
    void slotPreviousAutoDetectInputToggled(bool checked);
    void slotPreviousInputValueChanged(quint32 uni, quint32 ch);

protected:
    void updatePreviousInputSource();

protected:
    QKeySequence m_previousKeySequence;
    QLCInputSource *m_previousInputSource;

public slots:
    void accept();
};

/** @} */

#endif
