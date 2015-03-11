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
#include <QFileDialog>
#include <QTextCursor>
#include <QMessageBox>
#include <QFormLayout>
#include <QFileInfo>
#include <QAction>
#include <QDebug>
#include <QMenu>
#include <cmath>

#include "functionselection.h"
#include "channelsselection.h"
#include "assignhotkey.h"
#include "scripteditor.h"
#include "mastertimer.h"
#include "speeddial.h"
#include "script.h"
#include "doc.h"

ScriptEditor::ScriptEditor(QWidget* parent, Script* script, Doc* doc)
    : QWidget(parent)
    , m_script(script)
    , m_doc(doc)
    , m_lastUsedPath(QString())
{
    setupUi(this);
    initAddMenu();

    /* Name */
    m_nameEdit->setText(m_script->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));

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

    connect(m_testPlayButton, SIGNAL(clicked()), this, SLOT(slotTestRun()));
    connect(m_checkButton, SIGNAL(clicked()), this, SLOT(slotCheckSyntax()));

    connect(m_script, SIGNAL(stopped(quint32)), this, SLOT(slotFunctionStopped(quint32)));

    // Set focus to the editor
    m_nameEdit->setFocus();
}

ScriptEditor::~ScriptEditor()
{
    delete m_document;
    m_document = NULL;

    if (m_testPlayButton->isChecked() == true)
        m_script->stopAndWait();
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

    m_addSystemCommandAction = new QAction(QIcon(":/player_play.png"), tr("System Command"), this);
    connect(m_addSystemCommandAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSystemCommand()));

    m_addCommentAction = new QAction(QIcon(":/label.png"), tr("Comment"), this);
    connect(m_addCommentAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddComment()));

    m_addRandomAction = new QAction(QIcon(":/other.png"), tr("Random Number"), this);
    connect(m_addRandomAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddRandom()));

    m_addFilePathAction = new QAction(QIcon(":/fileopen.png"), tr("File Path"), this);
    connect(m_addFilePathAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddFilePath()));

    m_addMenu = new QMenu(this);
    m_addMenu->addAction(m_addStartFunctionAction);
    m_addMenu->addAction(m_addStopFunctionAction);
    //m_addMenu->addAction(m_addSetHtpAction);
    //m_addMenu->addAction(m_addSetLtpAction);
    m_addMenu->addAction(m_addSetFixtureAction);
    m_addMenu->addAction(m_addSystemCommandAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addWaitAction);
    //m_addMenu->addAction(m_addWaitKeyAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addCommentAction);
    m_addMenu->addAction(m_addRandomAction);
    m_addMenu->addAction(m_addFilePathAction);

    m_addButton->setMenu(m_addMenu);
}

QString ScriptEditor::getFilePath()
{
    /* Create a file open dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Open Executable File"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    QStringList filters;
#if defined(WIN32) || defined(Q_OS_WIN)
    filters << tr("All Files (*.*)");
#else
    filters << tr("All Files (*)");
#endif
    dialog.setNameFilters(filters);

    /* Append useful URLs to the dialog */
    QList <QUrl> sidebar;
    sidebar.append(QUrl::fromLocalFile(QDir::homePath()));
    sidebar.append(QUrl::fromLocalFile(QDir::rootPath()));
    dialog.setSidebarUrls(sidebar);

    if (!m_lastUsedPath.isEmpty())
        dialog.setDirectory(m_lastUsedPath);

    /* Get file name */
    if (dialog.exec() != QDialog::Accepted)
        return QString();

    QString fn = dialog.selectedFiles().first();
    if (fn.isEmpty() == true)
        return QString();

#if defined(WIN32) || defined(Q_OS_WIN)
    fn.replace("/", "\\");
#endif

    if (fn.contains(" "))
        return QString("\"%1\"").arg(fn);
    else
        return fn;
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

void ScriptEditor::slotFunctionStopped(quint32 id)
{
    if (id == m_script->id())
    {
        m_testPlayButton->blockSignals(true);
        m_testPlayButton->setChecked(false);
        m_testPlayButton->blockSignals(false);
    }
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
    QDialog dialog(this);
    // Use a layout allowing to have a label next to each field
    QVBoxLayout dLayout(&dialog);

    dLayout.addWidget(new QLabel(tr("Enter the desired time")));
    SpeedDial *sd = new SpeedDial(this);
    ushort dialMask = sd->visibilityMask();
    dialMask = (dialMask & ~SpeedDial::Infinite);
    dialMask = (dialMask & ~SpeedDial::Tap);
    sd->setVisibilityMask(dialMask);
    sd->setValue(1000);
    dLayout.addWidget(sd);

    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    dLayout.addWidget(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted)
    {
        m_editor->moveCursor(QTextCursor::StartOfLine);
        m_editor->textCursor().insertText(QString("%1:%2\n")
                              .arg(Script::waitCmd).arg(Function::speedToString(sd->value())));
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
    ChannelsSelection cfg(m_doc, this);
    if (cfg.exec() == QDialog::Rejected)
        return; // User pressed cancel

    QList<SceneValue> channelsList = cfg.channelsList();
    foreach(SceneValue sv, channelsList)
    {
        Fixture* fxi = m_doc->fixture(sv.fxi);
        if (fxi != NULL)
        {
            const QLCChannel* channel = fxi->channel(sv.channel);
            m_editor->moveCursor(QTextCursor::StartOfLine);
            m_editor->textCursor().insertText(QString("%1:%2 ch:%3 val:0 // %4, %5\n")
                                                .arg(Script::setFixtureCmd)
                                                .arg(fxi->id()).arg(sv.channel)
                                                .arg(fxi->name()).arg(channel->name()));
            m_editor->moveCursor(QTextCursor::Down);
        }
    }
}

void ScriptEditor::slotAddSystemCommand()
{
    QString fn = getFilePath();
    if (fn.isEmpty())
        return;

    QFileInfo fInfo(fn);
#if !defined(WIN32) && !defined(Q_OS_WIN)
    if (fInfo.isExecutable() == false)
    {
        QMessageBox::warning(this, tr("Invalid executable"), tr("Please select an executable file !"));
        return;
    }
#endif
    m_lastUsedPath = fInfo.absolutePath();

    QString args = QInputDialog::getText(this, tr("Enter the program arguments (leave empty if not required)"), "",
                                        QLineEdit::Normal, QString());

    QStringList argsList = args.split(" ");
    QString formattedArgs;
    foreach(QString arg, argsList)
    {
        formattedArgs.append(QString("arg:%1 ").arg(arg));
    }

    m_editor->moveCursor(QTextCursor::StartOfLine);
    m_editor->textCursor().insertText(QString("%1:%2 %3\n")
                                        .arg(Script::systemCmd)
                                        .arg(fn).arg(formattedArgs));
    m_editor->moveCursor(QTextCursor::Down);
}

void ScriptEditor::slotAddComment()
{
    bool ok = false;
    QString str = QInputDialog::getText(this, tr("Add Comment"), "",
                                        QLineEdit::Normal, QString(), &ok);
    if (ok == true)
    {
        //m_editor->moveCursor(QTextCursor::StartOfLine);
        m_editor->textCursor().insertText(QString("// %1").arg(str));
        //m_editor->moveCursor(QTextCursor::EndOfLine);
    }
}

void ScriptEditor::slotAddRandom()
{
    QDialog dialog(this);
    // Use a layout allowing to have a label next to each field
    QFormLayout dLayout(&dialog);

    dLayout.addRow(new QLabel(tr("Enter the range for the randomization")));

    QSpinBox *minSB = new QSpinBox(this);
    minSB->setRange(0, 999);
    QSpinBox *maxSB = new QSpinBox(this);
    maxSB->setRange(0, 999);
    maxSB->setValue(255);
    dLayout.addRow(tr("Minimum value"), minSB);
    dLayout.addRow(tr("Maximum value"), maxSB);

    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    dLayout.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted)
    {
        m_editor->moveCursor(QTextCursor::StartOfLine);
        m_editor->textCursor().insertText(QString("random(%1,%2)")
                              .arg(minSB->value()).arg(maxSB->value()));
        m_editor->moveCursor(QTextCursor::EndOfLine);
    }
}

void ScriptEditor::slotAddFilePath()
{
    QString fn = getFilePath();
    if (fn.isEmpty())
        return;

    QFileInfo fInfo(fn);
    m_lastUsedPath = fInfo.absolutePath();

    //m_editor->textCursor().insertText(QUrl::toPercentEncoding(fn));
    m_editor->textCursor().insertText(fn);
}

void ScriptEditor::slotCheckSyntax()
{
    QString errResult;
    QString scriptText = m_document->toPlainText();
    m_script->setData(scriptText);
    QList<int> errLines = m_script->syntaxErrorsLines();
    if (errLines.isEmpty())
    {
        errResult.append(tr("No syntax errors found in the script"));
    }
    else
    {
        QStringList lines = scriptText.split(QRegExp("(\r\n|\n\r|\r|\n)"), QString::KeepEmptyParts);
        foreach(int line, errLines)
        {
            errResult.append(tr("Syntax error at line %1:\n%2\n\n").arg(line).arg(lines.at(line - 1)));
        }
    }
    QMessageBox::information(this, tr("Script check results"), errResult);
}

void ScriptEditor::slotTestRun()
{
    if (m_testPlayButton->isChecked() == true)
        m_script->start(m_doc->masterTimer());
    else
        m_script->stopAndWait();
}
