#ifndef IDNOPTIMIZER_H
#define IDNOPTIMIZER_H

#include <QList>
#include <QSet>
#include <QPair>
#include <QByteArray>
#include <cstring>

#define IDN_ZERO_HOLD_FRAMES 5


class IdnOptimizer
{
    /*********************************************************************
    * Initialization
    *********************************************************************/
public:
    IdnOptimizer();
    ~IdnOptimizer();
    /*********************************************************************
    *
    *********************************************************************/
public:
    struct PacketInformation {
        QList<QPair<int, int> > ranges;
        int numberOfSingleChannels;
        int numberOfChannelPacks;
        int byteCount;
    };
    PacketInformation optimize(const QByteArray& data, const bool checkNullValues, int rangeBegin, int rangeEnd);
private:
    QList<int> changedValues(const QByteArray& oldData, const QByteArray& newData,  int rangeBegin, int rangeEnd);
    PacketInformation getRanges(QList<int> changed);
    QSet<int> changed;
    QByteArray oldData;
    quint16 zeroFrames[512];
};

#endif
