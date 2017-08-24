#include "nonclosablewidget.h"

NonClosableWidget::NonClosableWidget(QWidget *parent) : QWidget(parent)
{

}

void NonClosableWidget::closeEvent(QCloseEvent *event)
{
    event->ignore();
}
