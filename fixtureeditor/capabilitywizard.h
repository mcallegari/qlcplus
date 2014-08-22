/*
  Q Light Controller - Fixture Definition Editor
  capabilitywizard.h

  Copyright (C) Heikki Junnila

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

#ifndef CAPABILITYWIZARD_H
#define CAPABILITYWIZARD_H

#include <QDialog>

#include "ui_capabilitywizard.h"

class QLCCapability;
class QLCChannel;

/** @addtogroup fixtureeditor Fixture Editor
 * @{
 */

class CapabilityWizard : public QDialog, public Ui_CapabilityWizard
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    CapabilityWizard(QWidget* parent, const QLCChannel* channel);
    ~CapabilityWizard();

    /********************************************************************
     * Capabilities
     ********************************************************************/
public:
    const QList <QLCCapability*> capabilities() const {
        return m_caps;
    }

protected slots:
    void slotCreateCapabilities();

protected:
    QList <QLCCapability*> m_caps;
    const QLCChannel* m_channel;

};

/** @} */

#endif
