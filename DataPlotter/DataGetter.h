#ifndef DATAGETTER_H
#define DATAGETTER_H


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

#include <QtSql>             //  Включение определения классов модуля QtSql
#include <QSqlQueryModel>    //  Модель данных только для чтения для наборов результатов SQL


class IStrategy
{
public:
    virtual ~IStrategy() = default;
    virtual bool CheckFile(const QString &filePath) = 0;
    virtual QList<QPair<QString, qreal>> GetData(const QString &filePath) = 0;
};


class SQLiteStrategy : public IStrategy
{
    bool CheckFile(const QString &filePath)
    {
        openDatabase(filePath);
        QStringList tables = database.tables();            //  Возвращение списка таблиц базы данных, системных таблиц и представлений.
        if (tables.isEmpty()) {                            //  Проверка списка не содержание элементов
            qDebug() << "В БД отсутствуют таблицы";        //  Использование глобальной функции для записи пользовательской отладочной информации
            closeDatabase();
            return false;
        }
        closeDatabase();
        return true;
    }
    QList<QPair<QString, qreal>> GetData(const QString &filePath)
    {
        closeDatabase();
        openDatabase(filePath);
        QList<QPair<QString, qreal>> data;                 //  Создание списка пар значений строка - 64-битовое значение с плавающей точкой
        QStringList tables = database.tables();            //  Возвращение списка таблиц базы данных, системных таблиц и представлений.
        QString tableName = tables.first();                //  Возвращение ссылки на первый элемент (имя таблицы) в списке.
        QSqlQuery query;                                   //  Создание пустого запроса
        query.exec("SELECT * FROM " + tableName + " ");    //  Возвращает true и уст-ет состояние запроса в active если запрос SQL был успешным;
        while (query.next())                               //  Проверка доступности следующей записи, и размещение запроса в полученной записи.
        {
            QString Time = query.value(0).toString();      //  Возвращение строки со значением поля index в текущей записи.
            qreal Value = query.value(1).toReal();         //  Возвращение QVariant как qreal
            data.append(qMakePair(Time, Value));           //  Добавление в список пары значений (данных) в виде [дата, значение].
        }
        closeDatabase();
        return data;
    }

private:
    QSqlDatabase database;                                //  Пустой объект QSqlDatabase
    void openDatabase(const QString& filePath)
    {
        closeDatabase();
        database = QSqlDatabase::addDatabase("QSQLITE");  //  Создание соединения с базой данных
        database.setDatabaseName(filePath);               //  Установка имени базы данных соединения

        if (!database.open())                             //  Открытие соединения с базой данных, используя текущие значения соединения.
        {
            qDebug() << "Ошибка открытия базы данных:"    //  Возвращение информации о последней ошибке, произошедшей в базе данных.
                     << database.lastError().text();
        }
    }
    void closeDatabase()
    {
        if (database.isOpen())                            //  Проверка открытости соединения с базой данных в данный момент;
        {
            database.close();                             //  Закрытие соединения с базой данных, освобождая все полученные ресурсы и делая
        }                                                 //  недействительными все существующие QSqlQueryобъекты, используюемых базой данных.
    }
};


class JSONStrategy : public IStrategy
{
public:
    bool CheckFile(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))              //  Открытие файла с помощью перечисления QIODevice для открытия на чтения.
        {
            qDebug() << "Не удалось открыть файл:" << filePath; return false;
        }
        QByteArray jsonData = file.readAll();             //  Считывание всех данных с устройства и возвращение их в виде массива байтов.
        file.close();                                     //  Закрытие устройства и установка его OpenMode значение NotOpen
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);  //  Преобразование парсером из текста в QJsonDocument двоичное представление
        if (jsonDoc.isNull() || !jsonDoc.isArray())                 //  Проверка документа на нулевость и содержание массива
        {                                                           //  Нулевой документ создан через конструктор по умолчанию
            qDebug() << "Данный файл JSON некорректен"; return false;
        }
        QJsonArray jsonArray = jsonDoc.array();
        if (jsonArray.isEmpty())
        {
            qDebug() << "Данный JSON файл пуст!"; return false;
        }

        return true;
    }

    QList<QPair<QString, qreal>> GetData(const QString& filePath)
    {
        QFile file(filePath);
        file.open(QIODevice::ReadOnly);
        QByteArray jsonData = file.readAll();
        file.close();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        QJsonArray jsonArray = jsonDoc.array();
        QList<QPair<QString, qreal>> data;
        for (const QJsonValue& value : jsonArray)
        {
            if (value.isObject())
            {
                QJsonObject obj = value.toObject();
                QString Time = obj["Time"].toString();
                qreal Value = obj["Value"].toDouble();
                data.append(QPair<QString, qreal>(Time, Value));
            }
        }
        return data;
    }
};


/*class SCVStrategy : public IStrategy
{
public:
    bool CheckFile(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))              //  Открытие файла с помощью перечисления QIODevice для открытия на чтения.
        {
            qDebug() << "Не удалось открыть файл:" << filePath; return false;
        }
        QByteArray jsonData = file.readAll();             //  Считывание всех данных с устройства и возвращение их в виде массива байтов.
        file.close();                                     //  Закрытие устройства и установка его OpenMode значение NotOpen
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);  //  Преобразование парсером из текста в QJsonDocument двоичное представление
        if (jsonDoc.isNull() || !jsonDoc.isArray())                 //  Проверка документа на нулевость и содержание массива
        {                                                           //  Нулевой документ создан через конструктор по умолчанию
            qDebug() << "Данный файл JSON некорректен"; return false;
        }
        QJsonArray jsonArray = jsonDoc.array();
        if (jsonArray.isEmpty())
        {
            qDebug() << "Данный JSON файл пуст!"; return false;
        }

        return true;
    }

    QList<QPair<QString, qreal>> GetData(const QString& filePath)
    {
        QFile file(filePath);
        if ( !file.open(QFile::ReadOnly | QFile::Text) )
        {
                qDebug() << "File not exists";
            } else {
                // Создаём поток для извлечения данных из файла
                QTextStream in(&file);
                // Считываем данные до конца файла
                while (!in.atEnd())
                {
                    // ... построчно
                    QString line = in.readLine();
                    // учитываем, что строка разделяется точкой с запятой на колонки
                    for (QString item : line.split(";")) {
                        data.append(QPair<QString, qreal>(item, 1));
                    }
                }
                file.close();
            }
        return data;
    }
};*/


#endif // DATAGETTER_H
