#include <QApplication>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "main_window.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

//    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
//    db.setHostName("127.0.0.1");
//    db.setPort(3306);
//    db.setUserName("root");
//    db.setPassword("optimus27");
//    if (!db.open()) {
//        QMessageBox::critical(0, "Don't connected", QString("Cannot connect to database with the next settings:\n"
//                                                            "\thostname: '%1'\n"
//                                                            "\tport: '%2'\n"
//                                                            "\tusername: '%3'\n"
//                                                            "\tpassword: '%4'\n\n"
//                                                            "The error message: %5")
//                              .arg(db.hostName()).arg(db.port()).arg(db.userName()).arg(db.password()).arg(db.lastError().text()));
//    }

//    QSqlQuery query("SELECT * FROM gtes_starts.graphs_parameters_type;");
//    while (query.next()) {
//        int id = query.value(0).toInt();
//        QString symbol = query.value(1).toString();
//        QString fullname = query.value(2).toString();
//        QString units = query.value(3).toString().trimmed();
//        qDebug() << id << "\t" << symbol << "\t" << fullname << "\t" << units;
//    }

//    QSqlQuery query("SELECT * FROM gtes_starts.graphs_parameters_values;");
//    while (query.next()) {
//        int id = query.value(0).toInt();
//        int par_type_id = query.value(1).toInt();
//        QByteArray par_values = query.value(2).toByteArray().trimmed();
//        qDebug() << id << "\t" << par_type_id << "\t" << par_values;
//    }
//    qDebug() << "Last error:" << db.lastError().text();
//    db.close();

    return a.exec();
}
