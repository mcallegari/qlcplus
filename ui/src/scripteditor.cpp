/*
  Q Light Controller
  scripteditor.cpp

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

#include <QTextDocument>
#include <QInputDialog>
#include <QTextCursor>
#include <QAction>
#include <QDebug>
#include <QMenu>
#include <cmath>

#include "functionselection.h"
#include "fixtureselection.h"
#include "assignhotkey.h"
#include "scripteditor.h"
#include "mastertimer.h"
#include "script.h"
#include "doc.h"

ScriptEditor::ScriptEditor(QWidget* parent, Script* script, Doc* doc)
    : QWidget(parent)
    , m_script(script)
    , m_doc(doc)
{
    setupUi(this);
    initAddMenu();

    /* Name */
    m_nameEdit->setText(m_script->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(slotNameEdited(const QString&)));

    /* Document */
    m_document = new QTextDocument(m_script->data(), this);
    m_editor->setDocument(m_document);
    connect(m_document, SIGNAL(undoAvailable(bool)), m_undoButton, SLOT(setEnabled(bool)));
    m_document->setUndoRedoEnabled(false);
    m_document->setUndoRedoEnabled(true);
#if QT_VERSION >= 0x040700
    m_document->clearUndoRedoStacks();
#endif

    m_editor->moveCursor(QTextCursor::End);
    connect(m_document, SIGNAL(contentsChanged()), this, SLOT(slotContentsChanged()));

    // Set focus to the editor
    m_nameEdit->setFocus();
}

ScriptEditor::~ScriptEditor()
{
    delete m_document;
    m_document = NULL;
}

void ScriptEditor::initAddMenu()
{
    m_addStartFunctionAction = new QAction(QIcon(":/function.png"), tr("Start Function"), this);
    connect(m_addStartFunctionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddStartFunction()));

    m_addStopFunctionAction = new QAction(QIcon(":/fileclose.png"), tr("Stop Function"), this);
    connect(m_addStopFunctionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddStopFunction()));

    m_addWaitAction = new QAction(QIcon(":/speed.png"), tr("Wait"), this);
    connect(m_addWaitAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddWait()));

    m_addWaitKeyAction = new QAction(QIcon(":/key_bindings.png"), tr("Wait Key"), this);
    connect(m_addWaitKeyAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddWaitKey()));

    m_addSetHtpAction = new QAction(QIcon(":/fixture.png"), tr("Set HTP"), this);
    connect(m_addSetHtpAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSetHtp()));

    m_addSetLtpAction = new QAction(QIcon(":/fixture.png"), tr("Set LTP"), this);
    connect(m_addSetLtpAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSetLtp()));

    m_addSetFixtureAction = new QAction(QIcon(":/movinghead.png"), tr("Set Fixture"), this);
    connect(m_addSetFixtureAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSetFixture()));

    m_addCommentAction = new QAction(QIcon(":/label.png"), tr("Comment"), this);
    connect(m_addCommentAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddComment()));

    m_addMenu = new QMenu(this);
    m_addMenu->addAction(m_addStartFunctionAction);
    m_addMenu->addAction(m_addStopFunctionAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addWaitAction);
    m_addMenu->addAction(m_addWaitKeyAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addSetHtpAction);
    m_addMenu->addAction(m_addSetLtpAction);
    m_addMenu->addAction(m_addSetFixtureAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addCommentAction);

    m_addButton->setMenu(m_addMenu);
}

void ScriptEditor::slotNameEdited(const QString& name)
{
    m_script->setName(name);
}

void ScriptEditor::slotContentsChanged()
{
    //! @todo: this might become quite heavy if there's a lot of content
    m_script->setData(m_document->toPlainText());
    m_doc->setModified();
}

void ScriptEditor::slotAddStartFunction()
{
    FunctionSelection fs(this, m_doc);
    fs.setDisabledFunctions(QList <quint32> () << m_script->id());
    if (fs.exec() == QDialog::Accepted)
    {
        m_editor->moveCursor(QTextCursor::StartOfLine);
        QTextCursor cursor(m_editor->textCursor());

        foreach (quint32 id, fs.selection())
        {
            Function* function = m_doc->function(id);
            Q_ASSERT(function != NULL);
            QString cmd = QString("%1:%2 // %3\n").arg(Script::startFunctionCmd)
                                                   .arg(id)
                                                   .arg(function->name());
            cursor.insertText(cmd);
            m_editor->moveCursor(QTextCursor::Down);
        }
    }
}

void ScriptEditor::slotAddStopFunction()
{
    FunctionSelection fs(this, m_doc);
    fs.setDisabledFunctions(QList <quint32> () << m_script->id());
    if (fs.exec() == QDialog::Accepted)
    {
        m_editor->moveCursor(QTextCursor::StartOfLine);
        QTextCursor cursor(m_editor->textCursor());

        foreach (quint32 id, fs.selection())
        {
            Function* function = m_doc->function(id);
            Q_ASSERT(function != NULL);
            QString cmd = QString("%1:%2 // %3\n").arg(Script::stopFunctionCmd)
                                                  .arg(id)
                                                  .arg(function->name());
            cursor.insertText(cmd);
            m_editor->moveCursor(QTextCursor::Down);
        }
    }
}

void ScriptEditor::slotAddWait()
{
    bool ok = false;
    double val = QInputDialog::getDouble(this, tr("Wait"), tr("Seconds to wait"),
                                         1.0,     // Default
                                         0,       // Min
                                         INT_MAX, // Max
                                         3,       // Decimals
                                         &ok);
    if (ok == true)
    {
        m_editor->moveCursor(QTextCursor::StartOfLine);
        m_editor->textCursor().insertText(QString("%1:%2\n").arg(Script::waitCmd).arg(val));
    }
}

void ScriptEditor::slotAddWaitKey()
{
    AssignHotKey ahk(this);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_editor->moveCursor(QTextCursor::StartOfLine);
        m_editor->textCursor().insertText(QString("%1:%2 // Not supported yet\n")
                                                    .arg(Script::waitKeyCmd)
                                                    .arg(ahk.keySequence().toString()));
    }
}

void ScriptEditor::slotAddSetHtp()
{
    m_editor->moveCursor(QTextCursor::StartOfLine);
    m_editor->textCursor().insertText(QString("sethtp:0 val:0 uni:1 // Not supported yet\n"));
    m_editor->moveCursor(QTextCursor::EndOfLine);
}

void ScriptEditor::slotAddSetLtp()
{
    m_editor->moveCursor(QTextCursor::StartOfLine);
    m_editor->textCursor().insertText(QString("setltp:0 val:0 uni:1 // Not supported yet\n"));
    m_editor->moveCursor(QTextCursor::EndOfLine);
}

void ScriptEditor::slotAddSetFixture()
{
    FixtureSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    if (fs.exec() == QDialog::Accepted)
    {
        foreach (quint32 id, fs.selection())
        {
            Fixture* fxi = m_doc->fixture(id);
            if (fxi != NULL)
            {
                m_editor->moveCursor(QTextCursor::StartOfLine);
                m_editor->textCursor().insertText(QString("%1:%2 ch:0 val:0 // %2\n")
                                                    .arg(Script::setFixtureCmd)
                                                    .arg(fxi->id()).arg(fxi->name()));
                m_editor->moveCursor(QTextCursor::Down);
            }
        }
    }
}

void ScriptEditor::slotAddComment()
{
    bool ok = false;
    QString str = QInputDialog::getText(this, tr("Add Comment"), "",
                                        QLineEdit::Normal, QString(), &ok);
    if (ok == true)
    {
        m_editor->moveCursor(QTextCursor::StartOfLine);
        m_editor->textCursor().insertText(QString("// %1\n").arg(str));
        m_editor->moveCursor(QTextCursor::EndOfLine);
    }
}
