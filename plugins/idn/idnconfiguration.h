#ifndef IDNCONFIGURATION_H
#define IDNCONFIGURATION_H

#include <QUdpSocket>
#include <QTimer>
#include <QSet>

#include "ui_idnconfiguration.h"
#include "idnpacketizer.h"
#include "idn.h"

#define CONFIG_INTERFACE_SLOT 0
#define CONFIG_UNIVERSE_SLOT 1
#define CONFIG_UNITNAME_SLOT 2
#define CONFIG_SERVICENAME_SLOT 3
#define CONFIG_SERVICEID_SLOT 4
#define CONFIG_SERVICETYPE_SLOT 5
#define CONFIG_IPADDRESS_SLOT 6
#define CONFIG_PORT_SLOT 7
#define CONFIG_SENDMODE_SLOT 8
#define CONFIG_IDNCHANNEL_SLOT 9
#define CONFIG_RANGEBEGIN_SLOT 10
#define CONFIG_RANGEEND_SLOT 11


class IdnPlugin;

class IdnConfiguration : public QDialog, public Ui_IdnConfiguration
{
    Q_OBJECT

public:
    IdnConfiguration(IdnPlugin *plugin, QWidget *parent = 0);
    virtual ~IdnConfiguration();

public slots:
    int exec();
    void addReceiverSlot();

private slots:
    void on_m_buttonBox_accepted();

    void on_m_buttonBox_rejected();

    void on_m_scanButton_clicked();

    void waitForReply();

    void on_m_clientTree_customContextMenuRequested(const QPoint &pos);

    void deleteSlot();

    void on_m_addClientButton_clicked();

    void on_m_clearButton_clicked();

    void onScanComplete();

    void onScanAnimationTick();

private:
    //QHash<QHostAddress, QUdpSocket> m_scanSockets;
    IdnPacketizer *m_packetizer;
    void fillTree();
    int findDuplicates(QString ipAddress, int port, int serviceID);
    void uiSettings();
    void settings();
    bool validConfiguration;

    void sendScan(QUdpSocket *scanSocket, QHostAddress outputIP, int command);

    QString getServiceType(int serviceType);
    int getServiceTypeFromText(QString serviceType);

    QString scanItemKey(const IdnHostClientSettings &clientSettings);
    QString scanItemKey(QTreeWidgetItem *item);
    bool isManualItem(QTreeWidgetItem *item);
    void mergeScanResult(const IdnHostClientSettings &clientSettings);
    QTreeWidgetItem* createTreeItem(const IdnHostClientSettings &clientSettings);

private:
	IdnPlugin* m_plugin;
    QList<QUdpSocket*> activeSockets;
    QHash<QHostAddress, QString> unitNames;
    QTimer *m_scanTimer;
    QTimer *m_scanAnimationTimer;
    int m_scanAnimationStep;
    QSet<QString> m_staleScanKeys;
private:
    Ui::IdnConfiguration *ui;
};

#endif // IDNCONFIGURATION_H
