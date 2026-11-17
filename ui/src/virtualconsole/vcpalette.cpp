/*
  Q Light Controller Plus
  vcpalette.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDialogButtonBox>
#include <QListWidgetItem>
#include <QMutexLocker>
#include <QListWidget>
#include <QToolButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QPersistentModelIndex>
#include <QStylePainter>
#include <QStyleOptionToolButton>
#include <QLabel>
#include <QDebug>

#include "genericfader.h"
#include "fixturegroup.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "qlcchannel.h"
#include "vcpalette.h"
#include "universe.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

#define KXMLQLCVCPaletteFixture QStringLiteral("Fixture")
#define KXMLQLCVCPaletteGroup   QStringLiteral("Group")
#define KXMLQLCVCPaletteScene   QStringLiteral("Palette")
#define KXMLQLCVCPaletteColumns QStringLiteral("Columns")

static const QString buttonStyle =
    "QToolButton { border: 1px solid #555; border-radius: 4px; padding: 2px; background-color: %1; color: %2; }"
    "QToolButton:checked { border: 3px solid #FFC000; background-color: #3A6EA5; color: white; font-weight: bold; }"
    "QToolButton:pressed { border: 2px solid #FFC000; }";

VCPalette::VCPalette(QWidget* parent, Doc* doc)
    : VCWidget(parent, doc)
    , m_columns(6)
    , m_lastFixture(Fixture::invalidId())
    , m_valuesChanged(false)
{
    /* Set the class name "VCPalette" as the object name as well */
    setObjectName(VCPalette::staticMetaObject.className());

    setType(VCWidget::PaletteWidget);
    setCaption(tr("Palettes"));

    m_grid = new QGridLayout(this);
    m_grid->setContentsMargins(4, 4, 4, 4);
    m_grid->setSpacing(3);

    resize(QSize(420, 220));

    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved(quint32)));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixtureRemoved(quint32)));

    updateButtons();
}

VCPalette::~VCPalette()
{
    m_doc->masterTimer()->unregisterDMXSource(this);

    foreach (QSharedPointer<GenericFader> fader, m_fadersMap)
    {
        if (!fader.isNull())
            fader->requestDelete();
    }
    m_fadersMap.clear();
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCPalette::createCopy(VCWidget* parent) const
{
    Q_ASSERT(parent != NULL);

    VCPalette* palette = new VCPalette(parent, m_doc);
    if (palette->copyFrom(this) == false)
    {
        delete palette;
        palette = NULL;
    }

    return palette;
}

bool VCPalette::copyFrom(const VCWidget* widget)
{
    const VCPalette* palette = qobject_cast<const VCPalette*> (widget);
    if (palette == NULL)
        return false;

    m_fixtures = palette->m_fixtures;
    m_groups = palette->m_groups;
    m_palettes = palette->m_palettes;
    m_columns = palette->m_columns;
    updateButtons();

    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

QList<quint32> VCPalette::fixtures() const
{
    return m_fixtures;
}

void VCPalette::setFixtures(const QList<quint32> &ids)
{
    m_fixtures = ids;
}

QList<quint32> VCPalette::groups() const
{
    return m_groups;
}

void VCPalette::setGroups(const QList<quint32> &ids)
{
    m_groups = ids;
}

QList<quint32> VCPalette::palettes() const
{
    return m_palettes;
}

void VCPalette::setPalettes(const QList<quint32> &ids)
{
    m_palettes = ids;
}

int VCPalette::columns() const
{
    return m_columns;
}

void VCPalette::setColumns(int columns)
{
    m_columns = qMax(1, columns);
}

/*****************************************************************************
 * Buttons
 *****************************************************************************/

/** A tool button that wraps its text on multiple lines */
class PaletteButton final : public QToolButton
{
public:
    PaletteButton(QWidget *parent) : QToolButton(parent) { }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QStylePainter painter(this);
        QStyleOptionToolButton opt;
        initStyleOption(&opt);
        QString label = opt.text;
        opt.text.clear();
        painter.drawComplexControl(QStyle::CC_ToolButton, opt);

        QFont f = font();
        f.setBold(isChecked());
        painter.setFont(f);
        painter.setPen(isChecked() ? QColor(Qt::white) : palette().color(QPalette::ButtonText));
        painter.drawText(rect().adjusted(4, 2, -4, -2),
                         Qt::AlignCenter | Qt::TextWordWrap, label);
    }
};

QToolButton *VCPalette::createButton(const QString &text, bool checkable)
{
    QToolButton *btn = new PaletteButton(this);
    btn->setText(text);
    btn->setToolTip(text);
    btn->setCheckable(checkable);
    /* ignore the text size so that all the buttons get the same size */
    btn->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    btn->setMinimumSize(40, 28);
    btn->setStyleSheet(buttonStyle.arg("#E0E0E0", "black"));
    btn->setEnabled(mode() == Doc::Operate);
    btn->setAttribute(Qt::WA_TransparentForMouseEvents, mode() != Doc::Operate);
    btn->show();
    return btn;
}

void VCPalette::updateButtons()
{
    /* Remove all the existing buttons and labels */
    QLayoutItem *item;
    while ((item = m_grid->takeAt(0)) != NULL)
    {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    m_fixtureButtons.clear();
    m_groupButtons.clear();
    m_paletteButtons.clear();

    for (int r = 0; r < m_grid->rowCount(); r++)
        m_grid->setRowStretch(r, 0);
    for (int c = 0; c < m_grid->columnCount(); c++)
        m_grid->setColumnStretch(c, 0);
    for (int c = 0; c < m_columns; c++)
        m_grid->setColumnStretch(c, 1);

    int row = 0, col = 0;
    auto addToGrid = [&](QWidget *w)
    {
        m_grid->setRowStretch(row, 1);
        m_grid->addWidget(w, row, col);
        if (++col >= m_columns)
        {
            col = 0;
            row++;
        }
    };
    auto newSection = [&](const QString &title)
    {
        if (col != 0)
            row++;
        col = 0;
        QLabel *label = new QLabel(title, this);
        label->setStyleSheet("QLabel { font-weight: bold; }");
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_grid->addWidget(label, row++, 0, 1, m_columns);
    };

    /* Fixtures and groups */
    if (m_fixtures.isEmpty() == false || m_groups.isEmpty() == false)
    {
        newSection(tr("Fixtures"));

        foreach (quint32 id, m_groups)
        {
            FixtureGroup *grp = m_doc->fixtureGroup(id);
            if (grp == NULL)
                continue;
            QToolButton *btn = createButton(grp->name(), false);
            btn->setStyleSheet(buttonStyle.arg("#B8C8D8", "black"));
            connect(btn, SIGNAL(clicked()), this, SLOT(slotGroupClicked()));
            m_groupButtons[btn] = id;
            addToGrid(btn);
        }

        foreach (quint32 id, m_fixtures)
        {
            Fixture *fxi = m_doc->fixture(id);
            if (fxi == NULL)
                continue;
            QToolButton *btn = createButton(fxi->name(), true);
            btn->setChecked(m_selection.contains(id));
            connect(btn, SIGNAL(toggled(bool)), this, SLOT(slotFixtureToggled(bool)));
            m_fixtureButtons[btn] = id;
            addToGrid(btn);
        }
    }

    /* Palettes */
    if (m_palettes.isEmpty() == false)
    {
        newSection(tr("Palettes"));

        foreach (quint32 id, m_palettes)
        {
            Function *f = m_doc->function(id);
            if (f == NULL || f->type() != Function::SceneType)
                continue;
            QToolButton *btn = createButton(f->name(), false);
            QColor col = paletteColor(id);
            if (col.isValid())
                btn->setStyleSheet(buttonStyle.arg(col.name(), col.lightness() > 140 ? "black" : "white"));
            connect(btn, SIGNAL(clicked()), this, SLOT(slotPaletteClicked()));
            m_paletteButtons[btn] = id;
            addToGrid(btn);
        }
    }

    /* Commands */
    newSection(QString());
    QToolButton *btn = createButton(tr("All"), false);
    connect(btn, SIGNAL(clicked()), this, SLOT(slotSelectAll()));
    addToGrid(btn);
    btn = createButton(tr("None"), false);
    connect(btn, SIGNAL(clicked()), this, SLOT(slotSelectNone()));
    addToGrid(btn);
    btn = createButton(tr("Invert"), false);
    connect(btn, SIGNAL(clicked()), this, SLOT(slotInvertSelection()));
    addToGrid(btn);
    btn = createButton(tr("Release"), false);
    btn->setStyleSheet(buttonStyle.arg("#E8B0B0", "black"));
    connect(btn, SIGNAL(clicked()), this, SLOT(slotRelease()));
    addToGrid(btn);
    btn = createButton(tr("Release all"), false);
    btn->setStyleSheet(buttonStyle.arg("#D07070", "white"));
    connect(btn, SIGNAL(clicked()), this, SLOT(slotReleaseAll()));
    addToGrid(btn);
}

void VCPalette::updateSelectionButtons()
{
    QHashIterator<QToolButton*, quint32> it(m_fixtureButtons);
    while (it.hasNext())
    {
        it.next();
        it.key()->blockSignals(true);
        it.key()->setChecked(m_selection.contains(it.value()));
        it.key()->blockSignals(false);
    }
}

QColor VCPalette::paletteColor(quint32 sceneID) const
{
    Scene *scene = qobject_cast<Scene*>(m_doc->function(sceneID));
    if (scene == NULL)
        return QColor();

    int r = -1, g = -1, b = -1;
    quint32 fxiID = Fixture::invalidId();

    foreach (SceneValue scv, scene->values())
    {
        Fixture *fxi = m_doc->fixture(scv.fxi);
        if (fxi == NULL)
            continue;
        if (fxiID != Fixture::invalidId() && scv.fxi != fxiID)
            continue;

        const QLCChannel *ch = fxi->channel(scv.channel);
        if (ch == NULL || ch->group() != QLCChannel::Intensity)
            continue;

        if (ch->colour() == QLCChannel::Red)
            r = scv.value;
        else if (ch->colour() == QLCChannel::Green)
            g = scv.value;
        else if (ch->colour() == QLCChannel::Blue)
            b = scv.value;
        else
            continue;

        fxiID = scv.fxi;
    }

    if (r < 0 && g < 0 && b < 0)
        return QColor();

    return QColor(qMax(r, 0), qMax(g, 0), qMax(b, 0));
}

void VCPalette::slotFixtureToggled(bool checked)
{
    QToolButton *btn = qobject_cast<QToolButton*>(sender());
    if (btn == NULL || m_fixtureButtons.contains(btn) == false)
        return;

    quint32 id = m_fixtureButtons[btn];
    QList<quint32> range;
    range << id;

    /* Shift+click: apply the same state to all the fixtures in between */
    int from = m_fixtures.indexOf(m_lastFixture);
    int to = m_fixtures.indexOf(id);
    if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) && from >= 0 && to >= 0)
        range = m_fixtures.mid(qMin(from, to), qAbs(to - from) + 1);

    foreach (quint32 fxi, range)
    {
        if (checked)
            m_selection.insert(fxi);
        else
            m_selection.remove(fxi);
    }

    m_lastFixture = id;
    if (range.count() > 1)
        updateSelectionButtons();
}

void VCPalette::slotGroupClicked()
{
    QToolButton *btn = qobject_cast<QToolButton*>(sender());
    if (btn == NULL || m_groupButtons.contains(btn) == false)
        return;

    FixtureGroup *grp = m_doc->fixtureGroup(m_groupButtons[btn]);
    if (grp == NULL)
        return;

    QSet<quint32> ids;
    foreach (quint32 id, grp->fixtureList())
        ids.insert(id);

    /* If the whole group is already selected, deselect it, otherwise select it */
    if (m_selection.contains(ids))
        m_selection.subtract(ids);
    else
        m_selection.unite(ids);

    updateSelectionButtons();
}

void VCPalette::slotSelectAll()
{
    foreach (quint32 id, m_fixtureButtons.values())
        m_selection.insert(id);
    updateSelectionButtons();
}

void VCPalette::slotSelectNone()
{
    m_selection.clear();
    updateSelectionButtons();
}

void VCPalette::slotInvertSelection()
{
    foreach (quint32 id, m_fixtureButtons.values())
    {
        if (m_selection.contains(id))
            m_selection.remove(id);
        else
            m_selection.insert(id);
    }
    updateSelectionButtons();
}

void VCPalette::slotPaletteClicked()
{
    QToolButton *btn = qobject_cast<QToolButton*>(sender());
    if (btn == NULL || m_paletteButtons.contains(btn) == false)
        return;

    quint32 sceneID = m_paletteButtons[btn];

    QMutexLocker locker(&m_valuesMutex);
    foreach (quint32 fxiID, m_selection)
    {
        QMap<quint32, uchar> values = paletteValues(sceneID, fxiID);
        QMapIterator<quint32, uchar> it(values);
        while (it.hasNext())
        {
            it.next();
            m_values[fxiID][it.key()] = it.value();
        }
    }
    m_valuesChanged = true;
}

void VCPalette::slotRelease()
{
    QMutexLocker locker(&m_valuesMutex);
    foreach (quint32 fxiID, m_selection)
        m_values.remove(fxiID);
    m_valuesChanged = true;
}

void VCPalette::slotReleaseAll()
{
    QMutexLocker locker(&m_valuesMutex);
    m_values.clear();
    m_valuesChanged = true;
}

void VCPalette::slotFunctionRemoved(quint32 id)
{
    if (m_palettes.removeAll(id) > 0)
        updateButtons();
}

void VCPalette::slotFixtureRemoved(quint32 id)
{
    bool changed = m_fixtures.removeAll(id) > 0;
    m_selection.remove(id);

    {
        QMutexLocker locker(&m_valuesMutex);
        if (m_values.remove(id) > 0)
            m_valuesChanged = true;
    }

    if (changed)
        updateButtons();
}

/*****************************************************************************
 * Palette values
 *****************************************************************************/

/* Key used to match channels of different fixture models */
static QString channelKey(const QLCChannel *ch)
{
    if (ch->preset() != QLCChannel::Custom)
        return QString("p%1").arg(ch->preset());
    return QString("g%1c%2").arg(ch->group()).arg(ch->colour());
}

QMap<quint32, uchar> VCPalette::paletteValues(quint32 sceneID, quint32 fixtureID) const
{
    QMap<quint32, uchar> values;

    Scene *scene = qobject_cast<Scene*>(m_doc->function(sceneID));
    Fixture *target = m_doc->fixture(fixtureID);
    if (scene == NULL || target == NULL)
        return values;

    QList<SceneValue> sceneValues = scene->values();

    /* 1. The values of the fixture itself, if it is in the palette */
    foreach (SceneValue scv, sceneValues)
    {
        if (scv.fxi == fixtureID)
            values[scv.channel] = scv.value;
    }
    if (values.isEmpty() == false)
        return values;

    /* 2. The values of the first fixture of the same model and mode */
    quint32 sameModel = Fixture::invalidId();
    foreach (SceneValue scv, sceneValues)
    {
        Fixture *fxi = m_doc->fixture(scv.fxi);
        if (fxi == NULL)
            continue;
        if (sameModel == Fixture::invalidId() &&
            fxi->fixtureDef() != NULL && fxi->fixtureDef() == target->fixtureDef() &&
            fxi->fixtureMode() == target->fixtureMode())
            sameModel = scv.fxi;
        if (scv.fxi == sameModel && scv.channel < target->channels())
            values[scv.channel] = scv.value;
    }
    if (values.isEmpty() == false)
        return values;

    /* 3. Match channels by type */
    QHash<QString, uchar> byType;
    foreach (SceneValue scv, sceneValues)
    {
        Fixture *fxi = m_doc->fixture(scv.fxi);
        if (fxi == NULL)
            continue;
        const QLCChannel *ch = fxi->channel(scv.channel);
        if (ch == NULL)
            continue;
        QString key = channelKey(ch);
        if (byType.contains(key) == false)
            byType[key] = scv.value;
    }

    for (quint32 i = 0; i < target->channels(); i++)
    {
        const QLCChannel *ch = target->channel(i);
        if (ch == NULL)
            continue;
        QString key = channelKey(ch);
        if (byType.contains(key))
            values[i] = byType[key];
    }

    return values;
}

void VCPalette::writeDMX(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);

    QMutexLocker locker(&m_valuesMutex);

    if (m_valuesChanged == false)
        return;

    m_valuesChanged = false;

    foreach (QSharedPointer<GenericFader> fader, m_fadersMap)
    {
        if (!fader.isNull())
            fader->removeAll();
    }

    QMapIterator<quint32, QMap<quint32, uchar> > it(m_values);
    while (it.hasNext())
    {
        it.next();
        Fixture *fxi = m_doc->fixture(it.key());
        if (fxi == NULL)
            continue;

        quint32 universe = fxi->universe();
        if (universe >= (quint32)universes.count())
            continue;

        QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
        if (fader.isNull())
        {
            fader = universes[universe]->requestFader(Universe::Override);
            m_fadersMap[universe] = fader;
        }

        QMapIterator<quint32, uchar> chIt(it.value());
        while (chIt.hasNext())
        {
            chIt.next();
            FadeChannel *fc = fader->getChannelFader(m_doc, universes[universe], it.key(), chIt.key());
            if (fc->universe() == Universe::invalid())
            {
                fader->remove(fc);
                continue;
            }

            /* the palette always wins until it is released */
            fc->addFlag(FadeChannel::Override);
            fc->setFadeTime(0);
            fc->setStart(chIt.value());
            fc->setCurrent(chIt.value());
            fc->setTarget(chIt.value());
            fc->setReady(false);
            fc->setElapsed(0);
        }
    }
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

static QListWidget *checkList(QWidget *parent)
{
    QListWidget *list = new QListWidget(parent);
    list->setDragDropMode(QAbstractItemView::InternalMove);

    /* Shift+click on a checkbox applies the same state to all the items
       between the last clicked one and this one */
    QSharedPointer<QPersistentModelIndex> last(new QPersistentModelIndex());
    QObject::connect(list, &QListWidget::itemChanged, list, [list, last](QListWidgetItem *item)
    {
        int row = list->row(item);
        if (QApplication::keyboardModifiers() & Qt::ShiftModifier && last->isValid())
        {
            int from = qMin(row, last->row());
            int to = qMax(row, last->row());
            list->blockSignals(true);
            for (int i = from; i <= to; i++)
                list->item(i)->setCheckState(item->checkState());
            list->blockSignals(false);
            list->viewport()->update();
        }
        *last = QPersistentModelIndex(list->model()->index(row, 0));
    });

    return list;
}

static void addCheckItem(QListWidget *list, const QString &text, quint32 id, bool checked)
{
    QListWidgetItem *item = new QListWidgetItem(text, list);
    item->setData(Qt::UserRole, id);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

static QList<quint32> checkedIDs(QListWidget *list)
{
    QList<quint32> ids;
    for (int i = 0; i < list->count(); i++)
    {
        if (list->item(i)->checkState() == Qt::Checked)
            ids.append(list->item(i)->data(Qt::UserRole).toUInt());
    }
    return ids;
}

void VCPalette::editProperties()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Palette properties"));
    dialog.resize(480, 520);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QFormLayout *form = new QFormLayout();
    QLineEdit *captionEdit = new QLineEdit(caption(), &dialog);
    QSpinBox *columnsSpin = new QSpinBox(&dialog);
    columnsSpin->setRange(1, 32);
    columnsSpin->setValue(m_columns);
    form->addRow(tr("Name"), captionEdit);
    form->addRow(tr("Buttons per row"), columnsSpin);
    layout->addLayout(form);

    QTabWidget *tabs = new QTabWidget(&dialog);
    QListWidget *fxList = checkList(tabs);
    QListWidget *grpList = checkList(tabs);
    QListWidget *palList = checkList(tabs);
    tabs->addTab(fxList, tr("Fixtures"));
    tabs->addTab(grpList, tr("Fixture groups"));
    tabs->addTab(palList, tr("Palettes (scenes)"));
    layout->addWidget(tabs);

    /* Check/uncheck all the items of the current tab */
    QHBoxLayout *checkLayout = new QHBoxLayout();
    QPushButton *checkAllBtn = new QPushButton(tr("Select all"), &dialog);
    QPushButton *uncheckAllBtn = new QPushButton(tr("Deselect all"), &dialog);
    checkLayout->addWidget(checkAllBtn);
    checkLayout->addWidget(uncheckAllBtn);
    checkLayout->addStretch();
    layout->addLayout(checkLayout);

    auto setAllChecked = [tabs](Qt::CheckState state)
    {
        QListWidget *list = qobject_cast<QListWidget*>(tabs->currentWidget());
        if (list == NULL)
            return;
        list->blockSignals(true);
        for (int i = 0; i < list->count(); i++)
            list->item(i)->setCheckState(state);
        list->blockSignals(false);
        list->viewport()->update();
    };
    connect(checkAllBtn, &QPushButton::clicked, &dialog, [setAllChecked]() { setAllChecked(Qt::Checked); });
    connect(uncheckAllBtn, &QPushButton::clicked, &dialog, [setAllChecked]() { setAllChecked(Qt::Unchecked); });

    QLabel *hint = new QLabel(tr("Check the items to show. Drag them to change their order.\n"
                                 "A palette is a scene: set the values on one fixture of each model, "
                                 "only the channels in the scene are changed."), &dialog);
    hint->setWordWrap(true);
    layout->addWidget(hint);

    /* checked items first, in their current order */
    foreach (quint32 id, m_fixtures)
        if (Fixture *fxi = m_doc->fixture(id))
            addCheckItem(fxList, fxi->name(), id, true);
    foreach (Fixture *fxi, m_doc->fixtures())
        if (m_fixtures.contains(fxi->id()) == false)
            addCheckItem(fxList, fxi->name(), fxi->id(), false);

    foreach (quint32 id, m_groups)
        if (FixtureGroup *grp = m_doc->fixtureGroup(id))
            addCheckItem(grpList, grp->name(), id, true);
    foreach (FixtureGroup *grp, m_doc->fixtureGroups())
        if (m_groups.contains(grp->id()) == false)
            addCheckItem(grpList, grp->name(), grp->id(), false);

    foreach (quint32 id, m_palettes)
        if (Function *f = m_doc->function(id))
            addCheckItem(palList, f->name(), id, true);
    foreach (Function *f, m_doc->functionsByType(Function::SceneType))
        if (m_palettes.contains(f->id()) == false)
            addCheckItem(palList, f->name(), f->id(), false);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), &dialog, SLOT(reject()));
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted)
        return;

    setCaption(captionEdit->text());
    setColumns(columnsSpin->value());
    setFixtures(checkedIDs(fxList));
    setGroups(checkedIDs(grpList));
    setPalettes(checkedIDs(palList));
    updateButtons();
    m_doc->setModified();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCPalette::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCPalette)
    {
        qWarning() << Q_FUNC_INFO << "Fixture palette node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    m_fixtures.clear();
    m_groups.clear();
    m_palettes.clear();

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCPaletteFixture)
        {
            m_fixtures.append(root.readElementText().toUInt());
        }
        else if (root.name() == KXMLQLCVCPaletteGroup)
        {
            m_groups.append(root.readElementText().toUInt());
        }
        else if (root.name() == KXMLQLCVCPaletteScene)
        {
            m_palettes.append(root.readElementText().toUInt());
        }
        else if (root.name() == KXMLQLCVCPaletteColumns)
        {
            setColumns(root.readElementText().toInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown fixture palette tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    /* Functions and groups might be loaded after the Virtual Console,
       so build the buttons once everything is in place */
    QMetaObject::invokeMethod(this, "updateButtons", Qt::QueuedConnection);

    return true;
}

bool VCPalette::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCVCPalette);

    saveXMLCommon(doc);
    saveXMLWindowState(doc);
    saveXMLAppearance(doc);

    doc->writeTextElement(KXMLQLCVCPaletteColumns, QString::number(m_columns));
    foreach (quint32 id, m_groups)
        doc->writeTextElement(KXMLQLCVCPaletteGroup, QString::number(id));
    foreach (quint32 id, m_fixtures)
        doc->writeTextElement(KXMLQLCVCPaletteFixture, QString::number(id));
    foreach (quint32 id, m_palettes)
        doc->writeTextElement(KXMLQLCVCPaletteScene, QString::number(id));

    doc->writeEndElement();

    return true;
}

/*****************************************************************************
 * Mode
 *****************************************************************************/

void VCPalette::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        m_doc->masterTimer()->registerDMXSource(this);
    }
    else
    {
        m_doc->masterTimer()->unregisterDMXSource(this);

        {
            QMutexLocker locker(&m_valuesMutex);
            m_values.clear();
            m_valuesChanged = false;
        }

        foreach (QSharedPointer<GenericFader> fader, m_fadersMap)
        {
            if (!fader.isNull())
                fader->requestDelete();
        }
        m_fadersMap.clear();
        m_selection.clear();
    }

    foreach (QToolButton *btn, findChildren<QToolButton*>())
    {
        btn->setEnabled(mode == Doc::Operate);
        btn->setAttribute(Qt::WA_TransparentForMouseEvents, mode != Doc::Operate);
    }
    updateSelectionButtons();

    VCWidget::slotModeChanged(mode);
}
