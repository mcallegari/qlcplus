#ifndef TREEMODELITEM_H
#define TREEMODELITEM_H

#include <QStringList>

class TreeModel;

class TreeModelItem
{
public:
    TreeModelItem(QString label);
    ~TreeModelItem();

    QString label() const;
    void setLabel(QString label);

    void setData(QStringList data);

    QVariant data(int index);

    void setChildrenColumns(QStringList columns);

    void addChild(QString label, QStringList data, QString path = QString());

    bool hasChildren();

    TreeModel *children();

    void printItem(int tab = 0);

private:
    QString m_label;
    QStringList m_data;
    TreeModel *m_children;
};

#endif // TREEMODELITEM_H
