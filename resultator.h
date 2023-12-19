#ifndef RESULTATOR_H
#define RESULTATOR_H

#include <QWidget>
#include <QCloseEvent>
#include <QDebug>

namespace Ui {
class Resultator;
}

class Resultator : public QWidget
{
    Q_OBJECT

public:
    explicit Resultator(QWidget *parent = nullptr);
    ~Resultator();

private:
    Ui::Resultator *ui;
    void closeEvent(QCloseEvent *event);
    QString previous;
    QWidget *parent;

public slots:
    void receiver(QString);
};

#endif // RESULTATOR_H
