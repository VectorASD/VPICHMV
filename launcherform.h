#ifndef LAUNCHERFORM_H
#define LAUNCHERFORM_H

#include <QWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QMetaEnum>
#include <QSettings>
#include <QPushButton>
#include <QTextBrowser>

#include "mywidget.h"

namespace Ui {
    class LauncherForm;
}



enum Section { General, Network, Proxy };
enum Key { URI, Port, User, Password };

class SettingsWrapper { // Спасибо за демотивирующую статью https://habr.com/ru/articles/149085/ на использование QSettings
    Q_GADGET
    Q_ENUMS(Section)
    Q_ENUMS(Key)
    QMetaEnum keys;
    QMetaEnum sections;
    QSettings conf;
    QMap<QString, QVariant> defaults;
public:
    class ValueRef {
    public:
        ValueRef(SettingsWrapper &st, const QString &kp) : parent(st), keyPath(kp) {}
        ValueRef& operator = (const QVariant &d);
    private:
        SettingsWrapper &parent;
        const QString keyPath;
    };
private:
    SettingsWrapper() {
        const QMetaObject &mo = staticMetaObject;
        int idx = mo.indexOfEnumerator("Key");
        keys = mo.enumerator(idx);

        idx = mo.indexOfEnumerator("Section");
        sections = mo.enumerator(idx);
    }
    QString keyPath(Section s, Key k){
        auto szSection = sections.valueToKey(s);
        auto szKey = keys.valueToKey(k);
        return QString(s == General ? "%1" : "%2/%1").arg(szKey).arg(szSection);
    }
    static SettingsWrapper& instance(){
        static SettingsWrapper singleton;
        return singleton;
    }
public:
    static QVariant get(Key k, Section s) {
        SettingsWrapper &self = instance();
        QString key = self.keyPath(s, k);
        return self.conf.value(key, self.defaults[key]);
    }
    static ValueRef set(Key k, Section s) {
        SettingsWrapper &self = instance();
        return ValueRef(self, self.keyPath(s, k));
    }
    static void setDefaults(const QString &str) {
        SettingsWrapper &self = instance();

        //section/key : value
        //section - optional
        QRegExp rxRecord("^\\s*(((\\w+)/)?(\\w+))\\s*:\\s*([^\\s].{0,})\\b\\s*$");

        auto kvs = str.split(QRegExp(";\\W*"), QString::SkipEmptyParts); // На заметку себе возьму флаг SkipEmptyParts
        for (auto kv : kvs) {
            if (rxRecord.indexIn(kv) != -1) {
                QString section = rxRecord.cap(3);
                QString key = rxRecord.cap(4);
                QString value = rxRecord.cap(5);

                int iKey = self.keys.keyToValue(key.toLocal8Bit().data());
                if (iKey != -1){
                    int iSection = self.sections.keyToValue(section.toLocal8Bit().data());
                    if (section.isEmpty() || iSection != -1) {
                        self.defaults[rxRecord.cap(1)] = value;
                    }
                }
            }
        }
    }
};



class QWidgetImager: public QWidget {
    Imager* imgr;
    int id;
    MyWidget* parent;
public:
    QWidgetImager(MyWidget* parent, Imager* imager, int id) {
        auto paintTimer = new QTimer(this);
        connect(paintTimer, SIGNAL(timeout()), this, SLOT(repaint()));
        paintTimer->start(5);
        imgr = imager;
        this->id = id;
        this->parent = parent;
    }
    ~QWidgetImager() {
        parent->releaseImager(imgr);
    }

    void paintEvent(QPaintEvent *event) {
        //qDebug("PAINT Event: %u", event->type());
        auto r = event->rect();
        //qDebug("Rect: %d %d %d %d", r.x(), r.y(), r.width(), r.height());

        QPainter p;
        p.begin(this);
        if (id == parent->get_pet_id()) {
            p.setPen(QPen(Qt::blue));
            p.setBrush(QBrush(Qt::yellow));
            p.drawRect(r + QMargins(0, 0, -1, -1));
        }
        QImage img = imgr->noglThread(r.width(), r.height());
        p.drawImage(r, img);
        p.end();
    }
    void mousePressEvent(QMouseEvent *event) {
        parent->set_pet_id(id);
    }
};



class LauncherForm : public QWidget {
    Q_OBJECT

private:
    Ui::LauncherForm *ui;
    QGridLayout grid;
    QHBoxLayout hbox;
    QPushButton navigator;

    MyWidget MyEngine;
    QWidgetImager* imagers[25];
    QLineEdit pet_name = QLineEdit(this);

public:
    explicit LauncherForm(QApplication *parent);
    ~LauncherForm();
    void closeEvent(QCloseEvent *event);

private slots:
    void pet_name_changed(const QString &str);
};

#endif // LAUNCHERFORM_H
