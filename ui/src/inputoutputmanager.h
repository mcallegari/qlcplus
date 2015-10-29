/*
  Q Light Controller
  inputoutputmanager.h

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

#ifndef INPUTOUTPUTMANAGER_H
#define INPUTOUTPUTMANAGER_H

#include <QWidget>
#include <QIcon>

class InputOutputPatchEditor;
class QListWidgetItem;
class QListWidget;
class QSplitter;
class QLineEdit;
class QCheckBox;
class QToolBar;
class QTimer;
class QIcon;

class InputOutputMap;
class OutputPatch;
class InputPatch;
class Doc;

/** @addtogroup ui_io
 * @{
 */

class InputOutputManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(InputOutputManager)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    InputOutputManager(QWidget* parent, Doc* doc);
    virtual ~InputOutputManager();

    /** Get the singleton instance */
    static InputOutputManager* instance();

private:
    static InputOutputManager* s_instance;
    InputOutputMap* m_ioMap;

    /*************************************************************************
     * Tree widget
     *************************************************************************/
public slots:
    /** Update the input/output mapping list */
    void updateList();

private:
    /** Update the contents of the input universe to the item */
    void updateItem(QListWidgetItem *item, quint32 universe);

private slots:
    /** Listens to input data and displays a small icon to indicate a
        working connection between a plugin and an input device. */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /** Hides the small icon after a while */
    void slotTimerTimeout();

    /** Displays an editor for the currently selected universe */
    void slotCurrentItemChanged();

    /** Updates the current item */
    void slotMappingChanged();

    /** Destroy the current audio input instance */
    void slotAudioInputChanged();

    void slotAddUniverse();
    void slotDeleteUniverse();
    void slotUniverseNameChanged(QString name);
    void slotUniverseAdded(quint32 universe);
    void slotPassthroughChanged(bool checked);

protected:
    /** @reimp */
    void showEvent(QShowEvent* ev);

private:
    Doc *m_doc;
    QSplitter* m_splitter;
    QToolBar *m_toolbar;
    QAction* m_addUniverseAction;
    QAction* m_deleteUniverseAction;
    QLineEdit *m_uniNameEdit;
    QCheckBox *m_uniPassthroughCheck;
    QListWidget *m_list;
    QIcon m_icon;
    QTimer* m_timer;
    InputOutputPatchEditor *m_editor;
    quint32 m_editorUniverse;
};

/** @} */

#endif
