#include "launcherform.h"
#include "ui_launcherform.h"

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QtSql>

QString LoadTextFile(QString path, QString type) {
    QFile file(path);
    QString progress;
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        progress = stream.readAll();
        file.close();
    } else QMessageBox::warning(nullptr, "Открывашка", "Не удалось открыть " + type + "!");
    return progress;
}

void Styler(QApplication *app) {
    QString css = LoadTextFile(":/styles/style.css", "файл со стилями CSS");
    app->setStyleSheet(css);
}

void DataBaser() { // Спасибо https://habr.com/ru/articles/128836/ за код, что я всё равно серьёзно переоформил
    QTime t;
    t.start();
    QSqlDatabase sdb = QSqlDatabase::addDatabase("QSQLITE");
    sdb.setDatabaseName("../test.sqlite");
    qDebug("OPEN (elapsed): %u ms.", t.elapsed());
    if (!sdb.open()) {
        QMessageBox::warning(nullptr, "SQLite-Открывашка", QString("Не удалось открыть SQLite-bd (%1) :/").arg(sdb.lastError().text()));
        return;
    }

    t.start();
    QSqlQuery a_query; // не совсем понял, как оно подсоединилось к sdb-переменной... Видимо через Qt-контекст ;'-}
    bool ok = a_query.exec("CREATE TABLE my_table ("
      "number integer PRIMARY KEY NOT NULL, "
      "address VARCHAR(255), "
      "age integer"
    ");");
    if (!ok) qDebug("Вроде не удается создать таблицу, провертье карманы!");
    /*if (!ok) {
        QMessageBox::warning(nullptr, "SQLite-Открывашка", "Вроде не удается создать таблицу, провертье карманы!");
        // lastError ничего не даёт   (%1)").arg(sdb.lastError().text()));
        return;
    } можно игнорить ошибку, т.к. таблица просто *существует*   */
    qDebug("Table creation time: %u ms.", t.elapsed());

    t.start();
    ok = a_query.exec(QString("INSERT INTO my_table(number, address, age) "
      "VALUES (%1, '%2', %3);").arg("14").arg("hello world str.").arg("37"));
    if (!ok) qDebug() << "Кажется данные не вставляются, проверьте дверь, может она закрыта?";
    qDebug("Value insertion time: %u ms.", t.elapsed());

    t.start();
    if (!a_query.exec("SELECT * FROM my_table")) {
        QMessageBox::warning(nullptr, "SQLite-Открывашка", "Даже селект не получается, я пас...");
        return;
    }
    QSqlRecord rec = a_query.record();
    int number = 0, age = 0;
    QString address = "";
    while (a_query.next()) {
        number = a_query.value(rec.indexOf("number")).toInt();
        age = a_query.value(rec.indexOf("age")).toInt();
        address = a_query.value(rec.indexOf("address")).toString();
        qDebug("Reader time: %u ms.", t.elapsed());

        qDebug("number is %d. age is %d. address '%s'", number, age, address.toStdString().c_str());
    }
    /*
    Для новенькой bd:
        OPEN (elapsed): 14 ms.
        Table creation time: 91 ms.
        Value insertion time: 100 ms.
        Reader time: 0 ms.
        number is 14. age is 37. address 'hello world str.'

    Для уже заполненной bd:
        OPEN (elapsed): 14 ms.
        Вроде не удается создать таблицу, провертье карманы!
        Table creation time: 0 ms.
        Кажется данные не вставляются, проверьте дверь, может она закрыта?
        Value insertion time: 0 ms.
        Reader time: 1 ms.
        number is 14. age is 37. address 'hello world str.'

    Выводы: ожидал, конечно, что времени улетит больше, но, видимо, время спасло то, что в Values не так много данных...
        Не советую использовать в клиентских приложениях, где файл сохранения даже долю мегабайта (ну хотябы 16 кб)
        перевалить не может, как в текущей курсовой...
    */
}

void QSettingser() {
    QCoreApplication::setOrganizationName("Maurity");
    QCoreApplication::setApplicationName("VectorASDgameEngine");

    SettingsWrapper::setDefaults("Uri: blablalba; Network/Port: meow");
    SettingsWrapper::setDefaults("User: cat; Proxy/Password: woof");

    qDebug("value #1: %s", SettingsWrapper::get(Key::Password, Section::Proxy).toString().toStdString().c_str());
}

LauncherForm::LauncherForm(QApplication *app): QWidget(nullptr), ui(new Ui::LauncherForm) {
    ui->setupUi(this);
    setWindowTitle("Стартовое меню");
    resize(640, 640);

    app->installEventFilter(&MyEngine);
    MyEngine.setFocusPolicy(Qt::StrongFocus);
    MyEngine.show();

    for (int i = 0; i < 25; i++) imagers[i] = new QWidgetImager(&MyEngine, MyEngine.getImager(85 + i), i);

    setLayout(&grid);

    pet_name.setText(MyEngine.get_pet_name());
    connect(&pet_name, SIGNAL(textChanged(const QString&)), this, SLOT(pet_name_changed(const QString&)));

    hbox.addWidget(new QLabel("Pet name:", this));
    hbox.addWidget(&pet_name);
    hbox.addWidget(&navigator);
    navigator.setText("i");
    QFont font;
    font.setBold(true);
    navigator.setFont(font);
    navigator.setMaximumWidth(23);
    connect(&navigator, &QPushButton::clicked, [=]() {
        //QStringList list;
        //client(list, this, ip.text(), port.text().toUShort(), buffer, options2num());
        //result.setText(list.join(""));
        QTextBrowser *tb = new QTextBrowser();
        tb->setOpenExternalLinks(true);
        QString html = LoadTextFile(":/styles/qch/basic.html", "веб-документ навигатора HTML");
        tb->setHtml(html);
        tb->setWindowTitle("Навигатор");
        tb->show();
    });

    grid.addLayout(&hbox, 0,0, 1,5);

    for (int y = 0; y < 5; y++)
        for (int x = 0; x < 5; x++) {
            grid.addWidget(imagers[x + y * 5], y + 1, x);
        }

    Styler(app);
    //DataBaser();
    //QSettingser();

}

LauncherForm::~LauncherForm() {
    delete ui;
}

void LauncherForm::pet_name_changed(const QString& str) {
    QString str2 = str;
    MyEngine.set_pet_name(str2);
}

void LauncherForm::closeEvent(QCloseEvent *event) {
    for (int i = 0; i < 25; i++) delete imagers[i];
    event->accept();
}
