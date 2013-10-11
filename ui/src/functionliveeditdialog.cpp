/*
  Q Light Controller Plus
  functionliveeditdialog.cpp

  Copyright (c) Massimo Callegari

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
        case Function::Scene:
            m_editor = new SceneEditor(m_scrollArea, qobject_cast<Scene*> (func), m_doc, true);
        break;
        case Function::Chaser:
            m_editor = new ChaserEditor(m_scrollArea, qobject_cast<Chaser*> (func), m_doc);
        break;
        case Function::EFX:
            m_editor = new EFXEditor(m_scrollArea, qobject_cast<EFX*> (func), m_doc);
        break;
        case Function::RGBMatrix:
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
