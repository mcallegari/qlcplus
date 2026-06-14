#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QMenu>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>

#include "idnconfiguration.h"


IdnConfiguration::IdnConfiguration(IdnPlugin *plugin, QWidget *parent) : QDialog(parent), m_packetizer(new IdnPacketizer())
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;
    validConfiguration = true;
    setupUi(this);
    uiSettings();
    fillTree();

    m_scanTimer = new QTimer(this);
    m_scanTimer->setSingleShot(true);
    connect(m_scanTimer, &QTimer::timeout, this, &IdnConfiguration::onScanComplete);

    m_scanAnimationTimer = new QTimer(this);
    m_scanAnimationStep = 0;
    connect(m_scanAnimationTimer, &QTimer::timeout, this, &IdnConfiguration::onScanAnimationTick);
}

IdnConfiguration::~IdnConfiguration()
{
}

void IdnConfiguration::uiSettings()
{
    m_clientTree->setColumnWidth(CONFIG_INTERFACE_SLOT, 140);   // Iface
    m_clientTree->setColumnWidth(CONFIG_UNIVERSE_SLOT, 60);     // Universe
    m_clientTree->setColumnWidth(CONFIG_UNITNAME_SLOT, 140);    // Unit Name
    m_clientTree->setColumnWidth(CONFIG_SERVICENAME_SLOT, 140); // Service Name
    m_clientTree->setColumnWidth(CONFIG_SERVICEID_SLOT, 60);    // Service ID
    m_clientTree->setColumnWidth(CONFIG_SERVICETYPE_SLOT, 80);  // RecMode
    m_clientTree->setColumnWidth(CONFIG_IPADDRESS_SLOT, 140);   // IP
    m_clientTree->setColumnWidth(CONFIG_PORT_SLOT, 70);         // Port
    m_clientTree->setColumnWidth(CONFIG_SENDMODE_SLOT, 250);    // Mode
    m_clientTree->setColumnWidth(CONFIG_IDNCHANNEL_SLOT, 80);   // Channel ID
    m_clientTree->setColumnWidth(CONFIG_RANGEBEGIN_SLOT, 80);   // Range Begin
    m_clientTree->setColumnWidth(CONFIG_RANGEEND_SLOT, 80);     // Range End
}

void IdnConfiguration::fillTree()
{
    QHash<IdnHostInfo, IdnSettings> clientMap = m_plugin->getSetting();
    QHashIterator<IdnHostInfo, IdnSettings> it(clientMap);

    while (it.hasNext())
    {
        it.next();
        QTreeWidgetItem *pitem = new QTreeWidgetItem(m_clientTree);

        QSpinBox *universeSpin = new QSpinBox(this);
        universeSpin->setMinimum(0);
        quint32 universeValue = 0;
        if (it.value().disabled == false)
        {
            universeValue = it.value().universe;
        }
        universeSpin->setValue(universeValue);
        m_clientTree->setItemWidget(pitem, CONFIG_UNIVERSE_SLOT, universeSpin);

        if (it.value().scan)
        {
            pitem->setText(CONFIG_INTERFACE_SLOT, it.value().iface.toString());
            pitem->setText(CONFIG_IPADDRESS_SLOT, it.key().address.toString());         // IP
            pitem->setText(CONFIG_PORT_SLOT, QString::number(it.value().port)); // Port
            pitem->setText(CONFIG_UNITNAME_SLOT, it.value().unitName);          // Unit Name
            pitem->setText(CONFIG_SERVICENAME_SLOT, it.value().serviceName); // Service Name
            pitem->setText(CONFIG_SERVICEID_SLOT, QString::number(it.value().serviceID)); // Service ID
            pitem->setText(CONFIG_SERVICETYPE_SLOT, getServiceType(it.value().serviceType)); // Rec. Service Mode
        }
        else
        {
            QComboBox *ifaceCombo = new QComboBox(this);
            foreach (IdnOutput output, m_plugin->getOutputmapping())
            {
                ifaceCombo->addItem(output.address.ip().toString());
            }
            ifaceCombo->setCurrentText(it.value().iface.toString());
            m_clientTree->setItemWidget(pitem, CONFIG_INTERFACE_SLOT, ifaceCombo);

            QLineEdit *ipAddrEdit = new QLineEdit(it.key().address.toString(), this);
            m_clientTree->setItemWidget(pitem, CONFIG_IPADDRESS_SLOT, ipAddrEdit);

            QSpinBox *portSpin = new QSpinBox(this);
            portSpin->setRange(1000, 10000);
            portSpin->setValue(it.value().port);
            m_clientTree->setItemWidget(pitem, CONFIG_PORT_SLOT, portSpin);

            QLineEdit *unitNameEdit = new QLineEdit(it.value().unitName, this);
            m_clientTree->setItemWidget(pitem, CONFIG_UNITNAME_SLOT, unitNameEdit);

            QLineEdit *serviceNameEdit = new QLineEdit(it.value().serviceName, this);
            m_clientTree->setItemWidget(pitem, CONFIG_SERVICENAME_SLOT, serviceNameEdit);

            QSpinBox *serviceIDSpinBox = new QSpinBox(this);
            serviceIDSpinBox->setRange(0, 255);
            serviceIDSpinBox->setValue(it.value().serviceID);
            m_clientTree->setItemWidget(pitem, CONFIG_SERVICEID_SLOT, serviceIDSpinBox);

            QSpinBox *idnReceiveModeSpinBox = new QSpinBox(this);
            idnReceiveModeSpinBox->setRange(0, 7);
            idnReceiveModeSpinBox->setValue(it.value().serviceType);
            m_clientTree->setItemWidget(pitem, CONFIG_SERVICETYPE_SLOT, idnReceiveModeSpinBox); // Rec. Service Mode
        }

        QComboBox *combo = new QComboBox(this);
        combo->addItem(tr("Laser Projector Effects (Discrete)"));
        combo->addItem(tr("Laser Projector Effects (Discrete) - Optimized"));
        combo->addItem(tr("DMX512 (Discrete)"));
        combo->addItem(tr("DMX512 (Discrete) - Optimized"));

        switch (it.value().mode)
        {
        case 4:
            combo->setCurrentIndex(0);
            break;
        case 5:
            combo->setCurrentIndex(1);
            break;
        case 6:
            combo->setCurrentIndex(2);
            break;
        case 7:
            combo->setCurrentIndex(3);
            break;
        }
        m_clientTree->setItemWidget(pitem, CONFIG_SENDMODE_SLOT, combo); // Send Service Mode

        QSpinBox *idnChannelSpinBox = new QSpinBox(this);
        idnChannelSpinBox->setRange(0, 63);
        idnChannelSpinBox->setValue(it.value().idnChannel);
        m_clientTree->setItemWidget(pitem, CONFIG_IDNCHANNEL_SLOT, idnChannelSpinBox); // IDN Channel

        QSpinBox *rangeBeginEdit = new QSpinBox(this);
        rangeBeginEdit->setRange(1, 512);
        rangeBeginEdit->setValue(it.value().rangeBegin);
        m_clientTree->setItemWidget(pitem, CONFIG_RANGEBEGIN_SLOT, rangeBeginEdit); // Range Begin

        QSpinBox *rangeEndEdit = new QSpinBox(this);
        rangeEndEdit->setRange(1, 512);
        rangeEndEdit->setValue(it.value().rangeEnd);
        m_clientTree->setItemWidget(pitem, CONFIG_RANGEEND_SLOT, rangeEndEdit); // Range End
    }
}

int IdnConfiguration::findDuplicates(QString ipAddress, int port, int serviceID)
{
    QTreeWidgetItemIterator it(m_clientTree);
    QList<quint8> candidateList;

    QList<QTreeWidgetItem *> items = m_clientTree->findItems(ipAddress, Qt::MatchExactly, CONFIG_IPADDRESS_SLOT);

    foreach (QTreeWidgetItem *item, items)
    {
        int wPort, wServiceId;
        if (item->text(CONFIG_PORT_SLOT).isEmpty())
        {
            QSpinBox *portSpinBox = qobject_cast<QSpinBox *>(m_clientTree->itemWidget(item, CONFIG_PORT_SLOT));
            wPort = portSpinBox->value();
        }
        else
        {
            wPort = item->text(CONFIG_PORT_SLOT).toInt();
        }

        if(item->text(CONFIG_SERVICEID_SLOT).isEmpty()) {
            QSpinBox *serviceIDSpinBox = qobject_cast<QSpinBox *>(m_clientTree->itemWidget(item, CONFIG_SERVICEID_SLOT));
            wServiceId = serviceIDSpinBox->value();
        }
        else
        {
            wServiceId = item->text(CONFIG_SERVICEID_SLOT).toInt();
        }

        if (wPort == port && wServiceId == serviceID)
        {
            candidateList << 1;
        }
    }

    return candidateList.count();
}

int IdnConfiguration::exec()
{
    return QDialog::exec();
}

void IdnConfiguration::addReceiverSlot()
{
    QTreeWidgetItem *nitem = new QTreeWidgetItem(m_clientTree);

    QComboBox *ifaceCombo = new QComboBox(this);

    foreach (IdnOutput output, m_plugin->getOutputmapping())
    {
        ifaceCombo->addItem(output.address.ip().toString());
    }
    ifaceCombo->setCurrentIndex(ifaceCombo->count() - 1);
    m_clientTree->setItemWidget(nitem, CONFIG_INTERFACE_SLOT, ifaceCombo);

    QSpinBox *universeSpin = new QSpinBox(this);
    universeSpin->setMinimum(0);
    universeSpin->setValue(0);
    m_clientTree->setItemWidget(nitem, CONFIG_UNIVERSE_SLOT, universeSpin);

    QLineEdit *unitNameEdit = new QLineEdit("", this);
    m_clientTree->setItemWidget(nitem, CONFIG_UNITNAME_SLOT, unitNameEdit);

    QLineEdit *serviceNameEdit = new QLineEdit("", this);
    m_clientTree->setItemWidget(nitem, CONFIG_SERVICENAME_SLOT, serviceNameEdit);

    QSpinBox *serviceIDSpinBox = new QSpinBox(this);
    serviceIDSpinBox->setRange(0, 255);
    serviceIDSpinBox->setValue(0);
    m_clientTree->setItemWidget(nitem, CONFIG_SERVICEID_SLOT, serviceIDSpinBox);

    QLineEdit *ipAddrEdit = new QLineEdit("", this);
    m_clientTree->setItemWidget(nitem, CONFIG_IPADDRESS_SLOT, ipAddrEdit);

    QSpinBox *portSpin = new QSpinBox(this);
    portSpin->setRange(1000, 10000);
    portSpin->setValue(IDN_PORT);
    m_clientTree->setItemWidget(nitem, CONFIG_PORT_SLOT, portSpin);

    QSpinBox *serviceType = new QSpinBox(this);
    m_clientTree->setItemWidget(nitem, CONFIG_SERVICETYPE_SLOT, serviceType);

    QComboBox *combo = new QComboBox(this);
    combo->addItem(tr("Laser Projector Effects (Discrete)"));
    combo->addItem(tr("Laser Projector Effects (Discrete) - Optimized"));
    combo->addItem(tr("DMX512 (Discrete)"));
    combo->addItem(tr("DMX512 (Discrete) - Optimized"));
    m_clientTree->setItemWidget(nitem, CONFIG_SENDMODE_SLOT, combo);

    QSpinBox *idnChannelSpinBox = new QSpinBox(this);
    idnChannelSpinBox->setRange(0, 63);
    idnChannelSpinBox->setValue(0);
    m_clientTree->setItemWidget(nitem, CONFIG_IDNCHANNEL_SLOT, idnChannelSpinBox);

    QSpinBox *rangeBeginEdit = new QSpinBox(this);
    rangeBeginEdit->setRange(1, 512);
    rangeBeginEdit->setValue(1);
    m_clientTree->setItemWidget(nitem, CONFIG_RANGEBEGIN_SLOT, rangeBeginEdit);

    QSpinBox *rangeEndEdit = new QSpinBox(this);
    rangeEndEdit->setRange(1, 512);
    rangeEndEdit->setValue(512);
    m_clientTree->setItemWidget(nitem, CONFIG_RANGEEND_SLOT, rangeEndEdit);
}

void IdnConfiguration::on_m_buttonBox_accepted()
{
    QHash<IdnHostInfo, IdnSettings> settings;
    for (int i = 0; i < m_clientTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_clientTree->topLevelItem(i);
        IdnSettings tmp;
        if (item->text(CONFIG_INTERFACE_SLOT).isEmpty())
        {
            QComboBox *ifaceCombo = qobject_cast<QComboBox *>(m_clientTree->itemWidget(item, CONFIG_INTERFACE_SLOT));
            tmp.iface = QHostAddress(ifaceCombo->currentText());
            tmp.scan = false;
        }
        else
        {
            tmp.iface = QHostAddress(item->text(CONFIG_INTERFACE_SLOT));
            tmp.scan = true;
        }

        if (item->text(CONFIG_UNIVERSE_SLOT).isEmpty())
        {
            QSpinBox *universeSpinBox = qobject_cast<QSpinBox *>(m_clientTree->itemWidget(item, CONFIG_UNIVERSE_SLOT));
            if (universeSpinBox->value() == 0)
            {
                tmp.disabled = true;
                tmp.universe = 1;
            }
            else
            {
                tmp.disabled = false;
                tmp.universe = universeSpinBox->value();
            }
        }
        else
        {
            int universe = item->text(CONFIG_UNIVERSE_SLOT).toInt();
            if (universe == 0)
            {
                tmp.disabled = true;
                tmp.universe = 1;
            }
            else
            {
                tmp.disabled = false;
                tmp.universe = universe;
            }
        }

        if(item->text(CONFIG_UNITNAME_SLOT).isEmpty())
        {
            QLineEdit *unitNameEdit = qobject_cast<QLineEdit *>(m_clientTree->itemWidget(item, CONFIG_UNITNAME_SLOT));
            tmp.unitName = unitNameEdit->text();
        }
        else
        {
            tmp.unitName = item->text(CONFIG_UNITNAME_SLOT);
        }

        if(item->text(CONFIG_SERVICENAME_SLOT).isEmpty())
        {
            QLineEdit *serviceNameEdit = qobject_cast<QLineEdit *>(m_clientTree->itemWidget(item, CONFIG_SERVICENAME_SLOT));
            tmp.serviceName = serviceNameEdit->text();
        }
        else
        {
            tmp.serviceName = item->text(CONFIG_SERVICENAME_SLOT);
        }


        if (item->text(CONFIG_SERVICEID_SLOT).isEmpty())
        {
            QSpinBox *serviceIDSpinBox = qobject_cast<QSpinBox *>(m_clientTree->itemWidget(item, CONFIG_SERVICEID_SLOT));
            tmp.serviceID = serviceIDSpinBox->value();
        }
        else
        {
            tmp.serviceID = item->text(CONFIG_SERVICEID_SLOT).toInt();
        }

        QString ipStr;
        if (item->text(CONFIG_IPADDRESS_SLOT).isEmpty())
        {
            QLineEdit *ipAddrEdit = qobject_cast<QLineEdit *>(m_clientTree->itemWidget(item, CONFIG_IPADDRESS_SLOT));
            ipStr = ipAddrEdit->text();
        }
        else
        {
            ipStr = item->text(CONFIG_IPADDRESS_SLOT);
        }
        QHostAddress ip = QHostAddress(ipStr);

        if (ip.isNull())
        {
            QMessageBox::critical(this, tr("QLC+ IDN Plugin"), tr("You have to enter an valid IP-Address.\n"), QMessageBox::Cancel);
            return;
        }

        if (item->text(CONFIG_PORT_SLOT).isEmpty())
        {
            QSpinBox *portSpinBox = qobject_cast<QSpinBox *>(m_clientTree->itemWidget(item, CONFIG_PORT_SLOT));
            tmp.port = portSpinBox->value();
        }
        else
        {
            tmp.port = item->text(CONFIG_PORT_SLOT).toInt();
        }

        if (findDuplicates(ip.toString(), tmp.port, tmp.serviceID) > 1)
        {
            QMessageBox::critical(this, tr("QLC+ IDN Plugin"), tr("There are at least two receivers with the same IP-Address and Port.\n"), QMessageBox::Cancel);
            return;
        }

        if (item->text(CONFIG_SERVICETYPE_SLOT).isEmpty())
        {
            QSpinBox *serviceTypeSpinBox = qobject_cast<QSpinBox *>(m_clientTree->itemWidget(item, CONFIG_SERVICETYPE_SLOT));
            tmp.serviceType = serviceTypeSpinBox->value();
        }
        else
        {
            tmp.serviceType = getServiceTypeFromText(item->text(CONFIG_SERVICETYPE_SLOT));
        }

        QComboBox *modeCombo = qobject_cast<QComboBox *>(m_clientTree->itemWidget(item, CONFIG_SENDMODE_SLOT));
        tmp.mode = modeCombo->currentIndex() + 4;

        QSpinBox *idnChannelSpinBox = qobject_cast<QSpinBox *>(m_clientTree->itemWidget(item, CONFIG_IDNCHANNEL_SLOT));
        tmp.idnChannel = idnChannelSpinBox->value();

        QSpinBox *rangeBeginSpinBox = qobject_cast<QSpinBox *>(m_clientTree->itemWidget(item, CONFIG_RANGEBEGIN_SLOT));
        tmp.rangeBegin = rangeBeginSpinBox->value();

        QSpinBox *rangeEndSpinBox = qobject_cast<QSpinBox *>(m_clientTree->itemWidget(item, CONFIG_RANGEEND_SLOT));
        tmp.rangeEnd = rangeEndSpinBox->value();

        IdnHostInfo hostInfo;
        hostInfo.address = ip;
        hostInfo.serviceId = tmp.serviceID;

        settings[hostInfo] = tmp;
    }
    m_plugin->setSetting(settings);

    QDialog::accept();
}

void IdnConfiguration::deleteSlot()
{
    QList<QTreeWidgetItem *> itemList;
    itemList = m_clientTree->selectedItems();
    foreach (QTreeWidgetItem *item, itemList)
    {
        int index = m_clientTree->indexOfTopLevelItem(item);
        m_clientTree->takeTopLevelItem(index);
    }
}

void IdnConfiguration::on_m_buttonBox_rejected()
{
    QDialog::reject();
}

void IdnConfiguration::on_m_clearButton_clicked() {
    m_clientTree->clear();
}

void IdnConfiguration::onScanComplete()
{
    for (int i = m_clientTree->topLevelItemCount() - 1; i >= 0; --i)
    {
        QTreeWidgetItem *item = m_clientTree->topLevelItem(i);
        if (isManualItem(item))
            continue;
        if (m_staleScanKeys.contains(scanItemKey(item)))
            delete m_clientTree->takeTopLevelItem(i);
    }
    m_staleScanKeys.clear();

    m_scanAnimationTimer->stop();
    m_scanButton->setText(tr("Scan"));
    m_scanButton->setEnabled(true);
}

void IdnConfiguration::onScanAnimationTick()
{
    m_scanAnimationStep = (m_scanAnimationStep + 1) % 4;
    m_scanButton->setText(tr("Scanning") + QString(".").repeated(m_scanAnimationStep));
}

void IdnConfiguration::on_m_scanButton_clicked()
{
    if (m_scanTimer->isActive())
        return;

    m_scanButton->setEnabled(false);
    m_scanAnimationStep = 0;
    m_scanButton->setText(tr("Scanning"));
    m_scanAnimationTimer->start(350);

    m_staleScanKeys.clear();
    for (int i = 0; i < m_clientTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_clientTree->topLevelItem(i);
        if (!isManualItem(item))
            m_staleScanKeys.insert(scanItemKey(item));
    }
    m_scanTimer->start(3000);

    for (QUdpSocket *oldSocket : activeSockets)
    {
        oldSocket->disconnect();
        oldSocket->close();
        oldSocket->deleteLater();
    }
    activeSockets.clear();

    foreach (IdnOutput output, m_plugin->getOutputmapping())
    {
        QUdpSocket *socket = new QUdpSocket(this);
        bool bindSuccess = socket->bind(output.address.ip(), 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        activeSockets.append(socket);

        if (bindSuccess)
        {
            sendScan(socket, output.address.ip(), IDNCMD_SCANREQUEST);
            connect(socket, SIGNAL(readyRead()), this, SLOT(waitForReply()));
        }
    }
}

void IdnConfiguration::sendScan(QUdpSocket *socket, QHostAddress outputIP, int command)
{
    QByteArray scanRequestPacket;
    QHostAddress sendToIP = outputIP;
    if (command == IDNCMD_SCANREQUEST)
    {
        qDebug() << "[IDN] Sending scan request";
        m_packetizer->generateScanRequestPacket(scanRequestPacket);
        sendToIP = QHostAddress(QHostAddress::Broadcast);
    }
    else if (command == IDNCMD_SERVICEREQUEST)
    {
        qDebug() << "[IDN] Sending service map request";
        m_packetizer->generateServiceMapRequestPacket(scanRequestPacket);
    }
    else
    {
        qWarning() << "[IDN] sendScan: unknown command" << command;
    }
    qint64 sent = socket->writeDatagram(scanRequestPacket, sendToIP, IDN_PORT);
    if (sent < 0)
    {
        qWarning() << "[IDN] sendScan failed to" << sendToIP.toString() << ":" << IDN_PORT
                   << "error:" << socket->errorString();
    }
}

void IdnConfiguration::waitForReply()
{
    QUdpSocket *socket = qobject_cast<QUdpSocket *>(sender());
    QByteArray datagram;
    QHostAddress senderAddress;
    quint16 senderPort;

    while (socket->hasPendingDatagrams())
    {
        datagram.resize(socket->pendingDatagramSize());
        qint64 sent = socket->readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);
        qDebug() << QString("Packet from %1, size %2, data %3, port %4, iface %5")
                          .arg(senderAddress.toString())
                          .arg(sent)
                          .arg(datagram.toHex(' '))
                          .arg(senderPort)
                          .arg(socket->localAddress().toString());

        if (senderAddress == socket->localAddress() && senderAddress != QHostAddress(QString("127.0.0.1")))
        {
            qDebug() << "[IDN] Ignoring own packet from" << senderAddress.toString();
            continue;
        }

        if (m_packetizer->validateReply(datagram))
        {
            unitNames.insert(senderAddress, QString::fromUtf8(datagram.mid(24, 20).data()));
            sendScan(socket, senderAddress, IDNCMD_SERVICEREQUEST);
        }
        else if (m_packetizer->validateServiceMapReply(datagram))
        {
            int length = static_cast<unsigned char>(datagram[7]);
            for (int i = 0; i < length; i++)
            {
                int offset = 8 + i * 24;
                if(datagram.mid(offset, 1)[0] == 0x00)
                {
                    qWarning() << "ServiceID 0, ignore entry";
                    length++;
                    continue;
                }

                int serviceId = static_cast<unsigned char>(datagram.mid(offset, 1)[0]);
                QString serviceName = QString::fromUtf8(datagram.mid(offset + 4, 20).data());
                int serviceType = static_cast<unsigned char>(datagram.mid(offset + 1, 1)[0]);

                IdnHostClientSettings clientSettings;
                clientSettings.hostInfo.address = senderAddress;
                clientSettings.hostInfo.serviceId = serviceId;

                clientSettings.settings.port = IDN_PORT;
                clientSettings.settings.serviceName = serviceName;
                clientSettings.settings.unitName = unitNames.value(senderAddress);
                clientSettings.settings.serviceID = serviceId;
                clientSettings.settings.serviceType = serviceType;
                clientSettings.settings.iface = socket->localAddress();

                mergeScanResult(clientSettings);
            }
        }
        else
        {
            qWarning() << "Received invalid packet from " << senderAddress.toString();
        }
    }
}

void IdnConfiguration::on_m_clientTree_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);
    if (m_clientTree->selectedItems().count() != 0)
    {
        QAction *deleteClient = new QAction("Delete", this);
        connect(deleteClient, SIGNAL(triggered()), SLOT(deleteSlot()));
        menu->addAction(deleteClient);
        menu->popup(m_clientTree->mapToGlobal(pos));
    }
    else
    {
        QAction *scan = new QAction("Scan", this);
        connect(scan, SIGNAL(triggered()), SLOT(on_m_scanButton_clicked()));
        menu->addAction(scan);
        QAction *addClient = new QAction("Add new IDN Client", this);
        menu->addAction(addClient);
        connect(addClient, SIGNAL(triggered()), SLOT(addReceiverSlot()));
        menu->popup(m_clientTree->mapToGlobal(pos));
    }
}

void IdnConfiguration::on_m_addClientButton_clicked()
{
    addReceiverSlot();
}

QString IdnConfiguration::getServiceType(int serviceType)
{
    switch (serviceType)
    {
    case 0x00:
        return tr("Void");
    case 0x04:
        return tr("UART");
    case 0x05:
        return tr("DMX512");
    case 0x08:
        return tr("MIDI");
    case 0x80:
        return tr("Laser");
    case 0x84:
        return tr("Audio");
    default:
        return tr("Invalid");
    }
}

int IdnConfiguration::getServiceTypeFromText(QString serviceType)
{
    if (serviceType == QString("Void"))
        return 0x00;
    else if (serviceType == QString("UART"))
        return 0x04;
    else if (serviceType == QString("DMX512"))
        return 0x05;
    else if (serviceType == QString("MIDI"))
        return 0x08;
    else if (serviceType == QString("Laser"))
        return 0x80;
    else if (serviceType == QString("Audio"))
        return 0x84;
    else
        return 0;
}



QString IdnConfiguration::scanItemKey(const IdnHostClientSettings &clientSettings)
{
    return QString("%1_%2_%3_%4_%5")
        .arg(clientSettings.settings.unitName)
        .arg(clientSettings.settings.serviceName)
        .arg(clientSettings.settings.serviceID)
        .arg(clientSettings.settings.serviceType)
        .arg(clientSettings.hostInfo.address.toString());
}

QString IdnConfiguration::scanItemKey(QTreeWidgetItem *item)
{
    return QString("%1_%2_%3_%4_%5")
        .arg(item->text(CONFIG_UNITNAME_SLOT))
        .arg(item->text(CONFIG_SERVICENAME_SLOT))
        .arg(item->text(CONFIG_SERVICEID_SLOT))
        .arg(getServiceTypeFromText(item->text(CONFIG_SERVICETYPE_SLOT)))
        .arg(item->text(CONFIG_IPADDRESS_SLOT));
}

bool IdnConfiguration::isManualItem(QTreeWidgetItem *item)
{
    return qobject_cast<QComboBox *>(m_clientTree->itemWidget(item, CONFIG_INTERFACE_SLOT)) != nullptr;
}

void IdnConfiguration::mergeScanResult(const IdnHostClientSettings &clientSettings)
{
    QString key = scanItemKey(clientSettings);

    if (m_staleScanKeys.remove(key))
        return;

    for (int i = 0; i < m_clientTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_clientTree->topLevelItem(i);
        if (!isManualItem(item) && scanItemKey(item) == key)
            return;
    }

    createTreeItem(clientSettings);
}

QTreeWidgetItem* IdnConfiguration::createTreeItem(const IdnHostClientSettings &clientSettings) {
    QTreeWidgetItem *nitem = new QTreeWidgetItem(m_clientTree);

    nitem->setText(CONFIG_INTERFACE_SLOT, clientSettings.settings.iface.toString());

    QSpinBox *universeSpin = new QSpinBox(this);
    universeSpin->setMinimum(0);
    universeSpin->setValue(0);
    m_clientTree->setItemWidget(nitem, CONFIG_UNIVERSE_SLOT, universeSpin);

    nitem->setText(CONFIG_IPADDRESS_SLOT, clientSettings.hostInfo.address.toString());        
    nitem->setText(CONFIG_PORT_SLOT, QString::number(IDN_PORT));

    
    nitem->setText(CONFIG_SERVICEID_SLOT, QString::number(clientSettings.settings.serviceID));
    
    nitem->setText(CONFIG_SERVICENAME_SLOT, clientSettings.settings.serviceName);
    nitem->setText(CONFIG_UNITNAME_SLOT, unitNames.value(clientSettings.hostInfo.address));

    nitem->setText(CONFIG_SERVICETYPE_SLOT, getServiceType(clientSettings.settings.serviceType));

    QComboBox *combo = new QComboBox(this);
    combo->addItem(tr("Laser Projector Effects (Discrete)"));
    combo->addItem(tr("Laser Projector Effects (Discrete) - Optimized"));
    combo->addItem(tr("DMX512 (Discrete)"));
    combo->addItem(tr("DMX512 (Discrete) - Optimized"));
    m_clientTree->setItemWidget(nitem, CONFIG_SENDMODE_SLOT, combo);

    QSpinBox *idnChannelSpinBox = new QSpinBox(this);
    idnChannelSpinBox->setRange(0, 63);
    idnChannelSpinBox->setValue(0);
    m_clientTree->setItemWidget(nitem, CONFIG_IDNCHANNEL_SLOT, idnChannelSpinBox);

    QSpinBox *rangeBeginEdit = new QSpinBox(this);
    rangeBeginEdit->setRange(1, 512);
    rangeBeginEdit->setValue(1);
    m_clientTree->setItemWidget(nitem, CONFIG_RANGEBEGIN_SLOT, rangeBeginEdit);

    QSpinBox *rangeEndEdit = new QSpinBox(this);
    rangeEndEdit->setRange(1, 512);
    rangeEndEdit->setValue(512);
    m_clientTree->setItemWidget(nitem, CONFIG_RANGEEND_SLOT, rangeEndEdit);

    return nitem;
}
