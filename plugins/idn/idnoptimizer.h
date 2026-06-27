#ifndef IDNOPTIMIZER_H
#define IDNOPTIMIZER_H

#include <QList>
#include <QSet>
#include <QPair>
#include <QByteArray>


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
	struct PacketInformation{
		QList<QPair<int, int> > ranges;
		int numberOfSingleChannels;
		int numberOfChannelPacks;
		int byteCount;
	};
    PacketInformation optimize(const QByteArray& data, const bool checkNullValues);
private:
	QList<int> changedValues(QByteArray oldData, QByteArray newData);
	PacketInformation getRanges(QList<int> changed);
	QSet<int> changed;
 	QByteArray oldData;
    QList<int> nullChannelBuffer;
};

#endif
