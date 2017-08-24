#ifndef NONCLOSABLEWIDGET_H
#define NONCLOSABLEWIDGET_H

#include <QWidget>
#include <QCloseEvent>

class NonClosableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NonClosableWidget(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

signals:

public slots:
};

#endif // NONCLOSABLEWIDGET_H
