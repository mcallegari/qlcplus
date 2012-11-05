#include <QListWidget>
#include <QLayout>

#include "hotplugmonitor.h"
#include "hpmtest.h"

HPMTest::HPMTest(QWidget* parent)
    : QWidget(parent)
{
    new QHBoxLayout(this);
    m_list = new QListWidget(this);
    layout()->addWidget(m_list);

    HotPlugMonitor::connectListener(this);
}

HPMTest::~HPMTest()
{
}

void HPMTest::slotDeviceAdded(uint vid, uint pid)
{
    m_list->addItem(QString("%1: VID %2, PID %3").arg("Added")
                                                 .arg(QString::number(vid, 16))
                                                 .arg(QString::number(pid, 16)));
}

void HPMTest::slotDeviceRemoved(uint vid, uint pid)
{
    m_list->addItem(QString("%1: VID %2, PID %3").arg("Removed")
                                                 .arg(QString::number(vid, 16))
                                                 .arg(QString::number(pid, 16)));
}
