#include "resultator.h"
#include "ui_resultator.h"

Resultator::Resultator(QWidget *parent): QWidget(nullptr), ui(new Ui::Resultator) {
    ui->setupUi(this);
    // setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    this->parent = parent;
}

void Resultator::closeEvent(QCloseEvent *event) {
    if (parent->isVisible() && !parent->isHidden()) parent->close();
    event->accept();
}

Resultator::~Resultator() {
    delete ui;
}

void Resultator::receiver(QString data) {
    if (data == previous) return;
    previous = data;
    ui->textEdit->setText(data);
}
