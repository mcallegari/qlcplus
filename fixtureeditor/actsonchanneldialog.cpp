#include "qlcchannel.h"
#include "actsonchanneldialog.h"
#include "ui_actsonchanneldialog.h"

ActsOnChannelDialog::ActsOnChannelDialog(QList<QLCChannel *> allList, QVector<QLCChannel *> modeList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActsOnChannelDialog),
    m_channelsList(allList)
{
    ui->setupUi(this);

    ui->m_allTree->setIconSize(QSize(32, 32));
    ui->m_modeTree->setIconSize(QSize(32, 32));

    ui->m_allTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->m_allTree->setDragEnabled(true);
    ui->m_allTree->setDragDropMode(QAbstractItemView::InternalMove);
    ui->m_modeTree->setAcceptDrops(true);
    ui->m_modeTree->setDropIndicatorShown(true);
    ui->m_modeTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(ui->m_addChannel, SIGNAL(clicked()),
            this, SLOT(slotAddChannel()));
    connect(ui->m_removeChannel, SIGNAL(clicked()),
            this, SLOT(slotRemoveChannel()));

    fillChannelsTrees(m_channelsList, modeList);
}

ActsOnChannelDialog::~ActsOnChannelDialog()
{
    delete ui;
}

QList<QLCChannel *> ActsOnChannelDialog::getModeChannelsList()
{
    QList<QLCChannel *> retList;
    for (int i = 0; i < ui->m_modeTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = ui->m_modeTree->topLevelItem(i);
        if (item == NULL)
            continue;
        int idx = item->data(0, Qt::UserRole).toInt();
        if (idx < 0 || idx >= m_channelsList.count())
            continue;
        QLCChannel *ch = m_channelsList.at(idx);
        retList.append(ch);
    }
    return retList;
}

void ActsOnChannelDialog::slotAddChannel()
{
    QList<QTreeWidgetItem*> selection = ui->m_allTree->selectedItems();
    if (selection.count() == 0)
        return;

    foreach(QTreeWidgetItem *item, selection)
    {
        QTreeWidgetItem *newItem = item->clone();
        ui->m_modeTree->addTopLevelItem(newItem);
        ui->m_allTree->takeTopLevelItem(ui->m_allTree->indexOfTopLevelItem(item));
    }
}

void ActsOnChannelDialog::slotRemoveChannel()
{
    QList<QTreeWidgetItem*> selection = ui->m_modeTree->selectedItems();
    if (selection.count() == 0)
        return;

    foreach(QTreeWidgetItem *item, selection)
    {
        QTreeWidgetItem *newItem = item->clone();
        ui->m_allTree->addTopLevelItem(newItem);
        ui->m_modeTree->takeTopLevelItem(ui->m_modeTree->indexOfTopLevelItem(item));
    }
}

void ActsOnChannelDialog::fillChannelsTrees(QList<QLCChannel *> allList, QVector<QLCChannel *> modeList)
{
    int i = 0;
    foreach (QLCChannel *ch, allList)
    {
        if (modeList.contains(ch) == false)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(ui->m_allTree);
            item->setText(0, ch->name());
            item->setIcon(0, ch->getIcon());
            item->setData(0, Qt::UserRole, QVariant(i));
        }
        i++;
    }

    foreach (QLCChannel *ch, modeList)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->m_modeTree);
        int index = allList.indexOf(ch);
        item->setText(0, ch->name());
        item->setIcon(0, ch->getIcon());
        item->setData(0, Qt::UserRole, QVariant(index));
    }
}
