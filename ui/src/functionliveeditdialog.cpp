/*
  Q Light Controller Plus
  functionliveeditdialog.cpp

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

#include <QSettings>

#include "functionliveeditdialog.h"
#include "rgbmatrixeditor.h"
#include "chasereditor.h"
#include "sceneeditor.h"
#include "efxeditor.h"
#include "rgbmatrix.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#include "efx.h"

#define SETTINGS_GEOMETRY "funcliveedit/geometry"

FunctionLiveEditDialog::FunctionLiveEditDialog(Doc *doc, quint32 fid, QWidget *parent)
  : QDialog(parent)
  , m_doc(doc)
  , m_editor(NULL)
{
    Q_ASSERT(doc != NULL);

    Function *func = m_doc->function(fid);
    Q_ASSERT(func != NULL);

    setWindowTitle(tr("Function Live Edit"));
    setWindowIcon(QIcon(":/liveedit.png"));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());

    /* Master layout for toolbar and scroll area */
    new QVBoxLayout(this);
    setContentsMargins(0, 0, 0, 0);

    /* Scroll area that contains the editor widget */
    m_scrollArea = new QScrollArea(parent);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout()->addWidget(m_scrollArea);

    switch(func->type())
    {
        case Function::SceneType:
        {
            bool blindMode = true;
            if (func->isRunning())
                blindMode = false;
            SceneEditor *sceneEditor = new SceneEditor(m_scrollArea, qobject_cast<Scene*> (func), m_doc, true);
            sceneEditor->setBlindModeEnabled(blindMode);
            m_editor = sceneEditor;
        }
        break;
        case Function::ChaserType:
        case Function::SequenceType:
            m_editor = new ChaserEditor(m_scrollArea, qobject_cast<Chaser*> (func), m_doc, true);
        break;
        case Function::EFXType:
            m_editor = new EFXEditor(m_scrollArea, qobject_cast<EFX*> (func), m_doc);
        break;
        case Function::RGBMatrixType:
            m_editor = new RGBMatrixEditor(m_scrollArea, qobject_cast<RGBMatrix*> (func), m_doc);
        break;
        default:
        break;
    }

    if (m_editor != NULL)
    {
        m_scrollArea->setWidget(m_editor);
        m_editor->show();
        m_scrollArea->show();
    }
}

FunctionLiveEditDialog::~FunctionLiveEditDialog()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}
