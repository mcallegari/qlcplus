/*
  Q Light Controller
  launcher.h

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

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QStringList>
#include <QWidget>
#include <QEvent>

/**
 * Launcher is a very simple dialog for choosing whether to execute the main
 * QLC application or the Fixture Editor application.
 *
 * Once the choice has been made, the launcher window terminates and the chosen
 * application will be started. This kind of launcher is needed only for OSX
 * because application bundles can launch only one binary program.
 */
class Launcher : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(Launcher)

public:
    /**
     * Create a new launcher widget with a parent object.
     *
     * @param parent Owning parent widget (optional)
     */
    Launcher(QWidget* parent = 0);

    /** Destructor */
    ~Launcher();

private slots:
    /** Slot for fixture editor button click */
    void slotFXEDClicked();

    /** Slot for main application button click */
    void slotQLCClicked();

private:
    /** Launch Fixture Editor with given additional arguments */
    void launchFXED(const QStringList& arguments = QStringList());

    /** Launch Q Light Controller with given additional arguments */
    void launchQLC(const QStringList& arguments = QStringList());

protected:
    /** Acts as QApplication's event filter for QFileOpen events */
    bool eventFilter(QObject* object, QEvent* event);
};

#endif
