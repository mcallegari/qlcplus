/*
  Q Light Controller Plus
  vcmatrixproperties.h

  Copyright (c) Massimo Callegari

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

#ifndef VCMATRIXPROPERTIES_H
#define VCMATRIXPROPERTIES_H

#include <QDialog>

#include "qlcinputsource.h"
#include "vcmatrix.h"
#include "function.h"

#include "ui_vcmatrixproperties.h"

/** @addtogroup ui_vc_props
 * @{
 */

class VCMatrixProperties : public QDialog, public Ui_VCMatrixProperties
{
    Q_OBJECT
    Q_DISABLE_COPY(VCMatrixProperties)

public:
    explicit VCMatrixProperties(VCMatrix* button, Doc* doc);
    ~VCMatrixProperties();

protected slots:
    void slotAttachFunction();
    void slotSetFunction(quint32 fid = Function::invalidId());

    void slotAutoDetectInputToggled(bool checked);
    void slotInputValueChanged(quint32 universe, quint32 channel);
    void slotChooseInputClicked();

    void accept();

protected:
    void updateInputSource();

protected:
    VCMatrix* m_matrix;
    Doc* m_doc;

    quint32 m_function;
    QLCInputSource *m_inputSource;

};

/** @} */

#endif // VCMATRIXPROPERTIES_H
