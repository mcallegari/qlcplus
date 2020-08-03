#include "qlcchannel.h"
#include "actsonchanneldialog.h"
#include "ui_actsonchanneldialog.h"

ActsOnChannelDialog::ActsOnChannelDialog(QVector<QLCChannel*> allModeChannels, QHash<QLCChannel*, QLCChannel*> actsOnChannelsList, QLCChannel *currentChannel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActsOnChannelDialog),
    m_channelsList(allModeChannels),
    m_actsOnChannelsList(actsOnChannelsList)
{
    ui->setupUi(this);

    ui->m_allChannelsTree->setIconSize(QSize(32, 32));
    ui->m_actsOnTree->setIconSize(QSize(32, 32));

    ui->m_allChannelsTree->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->m_allChannelsTree->setDragEnabled(true);
    ui->m_allChannelsTree->setDragDropMode(QAbstractItemView::NoDragDrop);
    ui->m_actsOnTree->setAcceptDrops(true);
    ui->m_actsOnTree->setDropIndicatorShown(true);
    ui->m_actsOnTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(ui->m_addChannel, SIGNAL(clicked()),
            this, SLOT(slotAddChannel()));
    connect(ui->m_removeChannel, SIGNAL(clicked()),
            this, SLOT(slotRemoveChannel()));

    fillChannelsTrees(allModeChannels, currentChannel);
}

ActsOnChannelDialog::~ActsOnChannelDialog()
{
    delete ui;
}

QLCChannel *ActsOnChannelDialog::getModeChannelActsOn()
{
    QLCChannel *actsOnChannel = nullptr;

    for(int i = 0; i < ui->m_actsOnTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = ui->m_actsOnTree->topLevelItem(i);
        if (item == NULL)
            continue;
        int idx = item->data(0, Qt::UserRole).toInt();
        if (idx < 0 || idx >= m_channelsList.count())
            continue;

        actsOnChannel = m_channelsList.at(idx);
    }

    return actsOnChannel;
}

void ActsOnChannelDialog::slotAddChannel()
{
    QList<QTreeWidgetItem*> selection = ui->m_allChannelsTree->selectedItems();
    if(selection.count() == 1 && ui->m_actsOnTree->topLevelItemCount() == 0)
    {
        foreach(QTreeWidgetItem *item, selection)
        {
            QTreeWidgetItem *newItem = item->clone();
            ui->m_actsOnTree->addTopLevelItem(newItem);
            ui->m_allChannelsTree->takeTopLevelItem(ui->m_allChannelsTree->indexOfTopLevelItem(item));
        }
    }else{
        return;
    }
}

void ActsOnChannelDialog::slotRemoveChannel()
{
    QList<QTreeWidgetItem*> selection = ui->m_actsOnTree->selectedItems();
    if (selection.count() == 0)
        return;

    foreach(QTreeWidgetItem *item, selection)
    {
        QTreeWidgetItem *newItem = item->clone();
        ui->m_allChannelsTree->addTopLevelItem(newItem);
        ui->m_actsOnTree->takeTopLevelItem(ui->m_actsOnTree->indexOfTopLevelItem(item));
    }
}

void ActsOnChannelDialog::fillChannelsTrees(QVector<QLCChannel *> allModeChannels, QLCChannel *currentChannel)
{
    ui->m_actsOnTree->clear();

    QLCChannel *actsOnChannel = m_actsOnChannelsList.value(currentChannel, nullptr);

    int i = 0;
    foreach (QLCChannel *channel, allModeChannels)
    {
        if (currentChannel != channel && actsOnChannel != channel)
        {
            QTreeWidgetItem *itemChannel = new QTreeWidgetItem(ui->m_allChannelsTree);
            itemChannel->setText(0, channel->name());
            itemChannel->setIcon(0, channel->getIcon());
            itemChannel->setData(0, Qt::UserRole, QVariant(i));
        }
        i++;
    }

    if(actsOnChannel != nullptr)
    {
        QTreeWidgetItem *itemActsOn = new QTreeWidgetItem(ui->m_actsOnTree);

        int index = allModeChannels.indexOf(actsOnChannel);
        itemActsOn->setText(0, actsOnChannel->name());
        itemActsOn->setIcon(0, actsOnChannel->getIcon());
        itemActsOn->setData(0, Qt::UserRole, QVariant(index));
    }
}
