#ifndef DATAPLOTTER_H
#define DATAPLOTTER_H


#include <QString>
#include <QList>

#include <QMainWindow>
#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTableView>

#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QTableView>
#include <QHeaderView>
#include <QStatusBar>

#include <QtSql>           //  Подключение библиотеки для работы с SQLITE
#include <QSqlQueryModel>  //  Подключение библиотеки для работы с JSON


class IStrategy
{
public:
    virtual ~IStrategy() = default;
    virtual bool CheckFile(const QString &filePath) = 0;
    virtual QList<QPair<QString, qreal>> GetData(const QString &filePath) = 0;

};


class SQLiteStrategy : public IStrategy
{
    bool CheckFile(const QString &filePath) {}
    // Пока что за процесс компоновки данных отвечает функция получения данных
    // Так мы обрабатываем данные при их получении и занимаем меньше пространства в памяти при хранении этих данных
    // Но стоит ли компоновку данных выносить в отдельную функцию?
    QList<QPair<QString, qreal>> GetData(const QString &filePath)
    { }

private:
    QSqlDatabase database;
    void openDatabase(const QString& filePath)
    {}
    void closeDatabase() {   if (database.isOpen()) {   database.close();   }   }
};


class JSONStrategy : public IStrategy
{
public:
    bool CheckFile(const QString& filePath)
    {}

    QList<QPair<QString, qreal>> GetData(const QString& filePath)
    {}
};


#endif // DATAGETTER_H
#endif // DATAPLOTTER_H
