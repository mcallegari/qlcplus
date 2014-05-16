#include <QItemDelegate>
#include <QComboBox>

#include <stdio.h>

class VCSpeedDialFunctionDelegate : public QItemDelegate
{
Q_OBJECT

public:
    VCSpeedDialFunctionDelegate(QWidget *parent = 0) : QItemDelegate(parent) {
        printf("ctor\n");
    }
    virtual ~VCSpeedDialFunctionDelegate()
    {
        printf( "dtor\n");
    }

    QWidget *createEditor(QWidget *parent,
            const QStyleOptionViewItem &/* option */,
            const QModelIndex &/* index */) const
    {
        printf("nu ed\n");
        QComboBox *comboBox = new QComboBox(parent);

        comboBox->addItem("None");
        comboBox->addItem("1/16");
        comboBox->addItem("1/8");
        comboBox->addItem("1/4");
        comboBox->addItem("1/2");
        comboBox->addItem("1");
        comboBox->addItem("2");
        comboBox->addItem("4");
        comboBox->addItem("8");
        comboBox->addItem("16");

        return comboBox;
    }

    void setEditorData(QWidget *editor,
            const QModelIndex &index) const
    {
        printf("set ed data\n");
        int value = index.model()->data(index, Qt::UserRole).toInt();
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex(value);

        //QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        //m_fadeInCombo->setCurrentIndex(dial->fadeInMultiplier());
        //spinBox->setValue(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
            const QModelIndex &index) const
    {
        printf("set model data\n");
        //QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int value = comboBox->currentIndex();

        model->setData(index, value, Qt::UserRole);
        model->setData(index, comboBox->currentText(), Qt::DisplayRole);

        //spinBox->interpretText();
        //int value = spinBox->value();

        //model->setData(index, value, Qt::EditRole);
    }


    void updateEditorGeometry(QWidget *editor,
            const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
    {
        printf("update ed geom\n");
        editor->setGeometry(option.rect);
    }

//void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    (void)painter;
//    (void)option;
//    (void)index;
//}


};
