#include <QStyledItemDelegate>
#include <QComboBox>

#include "vcspeeddialfunction.h"

class VCSpeedDialFunctionDelegate : public QStyledItemDelegate
{
Q_OBJECT

public:
    VCSpeedDialFunctionDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {

    }

    QWidget *createEditor(QWidget *parent,
            const QStyleOptionViewItem &/*option*/,
            const QModelIndex &/*index*/) const
    {
        QComboBox *comboBox = new QComboBox(parent);

        comboBox->addItems(VCSpeedDialFunction::speedMultiplierNames());

        return comboBox;
    }

    void setEditorData(QWidget *editor,
            const QModelIndex &index) const
    {
        int value = index.model()->data(index, Qt::UserRole).toInt();
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
            const QModelIndex &index) const
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int value = comboBox->currentIndex();

        model->setData(index, value, Qt::UserRole);
        model->setData(index, comboBox->currentText(), Qt::DisplayRole);
    }


    void updateEditorGeometry(QWidget *editor,
            const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
    {
        editor->setGeometry(option.rect);
    }

};
