#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class TreeModelItem;

class TreeModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(TreeModel)
public:
    enum FixedRoles {
        LabelRole = Qt::UserRole + 1,
        ItemsCountRole,
        HasChildrenRole,
        ChildrenModel,
        FixedRolesEnd
    };

    TreeModel(QObject *parent = 0);
    ~TreeModel();

    void clear();

    void setColumnNames(QStringList names);

    void addItem(QString label, QStringList data, QString path = QString());

    Q_INVOKABLE int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    void printTree(int tab = 0);

protected:
    QStringList m_roles;
    QHash<int, QByteArray> roleNames() const;
    QList<TreeModelItem *> m_items;
    QMap<QString, TreeModelItem *> m_itemsPathMap;
};

#endif // TREEMODEL_H
