#ifndef DATAPLOTTER_H
#define DATAPLOTTER_H


#include <QBarSeries>           //  Библиотека для представления серии данных в виде вертикальных полос, сгруппированных по категориям
#include <QtCharts>             //  Модуль для представления набора простых в использовании компонентов диаграммы
#include <QtCharts/QChartView>  //  Библиотека для отдельного виджета, способного отображать диаграммы
#include <QtCharts/QPieSeries>  //  Библиотека для представления данных в виде круговых диаграмм
#include <QtCharts/QPieSlice>   //  Библиотека для представления одного среза в серии круговых диаграмм
#include <QPainter>             //  Библиотека для низкоуровневого рисования виджетов и других устройств рисования
#include <QPdfWriter>           //  Библиотека для создания PDF-файлов, используемых в качестве устройства рисования.


class DataPlotter
{
public:
    void DrawChart(QChartView* chartView, const QList<QPair<QString, qreal>>& data)  //   Сам Шаблонный метод
    {
        chartView->chart()                      //  Возвращение указателя (на объект диаграммы) типа QChart* на связанную диаграмму.
        ->removeAllSeries();                    //  Очистка окна рисования (Удаление всех объектов серии, добавленых на диаграмму).
        SetupChart(chartView, data);            //  Подготовка данных (закладывание их в chartView) и настройка диаграммы нужного типа
        chartView->update();                    //  Обновление окна вывода
    };

protected:
    virtual void SetupChart(QChartView* chartView, const QList<QPair<QString, qreal>>& data) = 0;  //  Абстрактная операция для шаблонного метода
};


class PieDataPlotter : public DataPlotter  //  Класс для создания круговых диаграмм
{
protected:
    void SetupChart(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
    {
        QPieSeries *series = new QPieSeries();  //  Создание объекта пустой серии
        for (int i = 0; i < 10; ++i)
        {
            const QPair<QString, qreal>& pair = data[i];
            QString time = pair.first;
            qreal value = pair.second;
            series->append(time, value);  //  Добавление срезов к серии
        }
        chartView->chart()->addSeries(series);
        //chartView->chart()->legend()->show();
        chartView->chart()->setTitle("Круговая диаграмма");
        chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);
        chartView->setRenderHint(QPainter::Antialiasing);
    }
};


class BarDataPlotter : public DataPlotter  //  Класс для создания столбчатых диаграмм
{
protected:
    void SetupChart(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
    {
        QBarSeries *series = new QBarSeries();     //   Создание серии пустых столбцов
        for (int i = 0; i < 10; ++i)
        {
            const QPair<QString, qreal>& pair = data[i];
            QString time = pair.first;
            qreal value = pair.second;
            QBarSet *barSet = new QBarSet(time);   //  Создание набора данных с меткой time.
            *barSet << value;                      //  Добавление данных к набору с помощью оператора <<.
            series->append(barSet);                //  Добавление наборов данных к серии
        }
        chartView->chart()->addSeries(series);     //  Добавление серии к диаграмме
        //chartView->chart()->legend()->show();
        chartView->chart()->setTitle("Столбчатая диаграмма");               //  Определение заголовка диаграммы.
        chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);  //  Определение параметров анимации для диаграммы (Включена анимация серий).
        chartView->setRenderHint(QPainter::Antialiasing);                   //  Установка данный флага на художника
    }                                                                       //  (Движок должен сглаживать края примитивов, если это возможно).
};

class LineDataPlotter : public DataPlotter  //  Класс для создания круговых диаграмм
{
protected:
    void SetupChart(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
    {
        QLineSeries *series = new QLineSeries();  //  Создание объекта пустой серии
        for (int i = 0; i < 10; ++i)
        {
            const QPair<QString, qreal>& pair = data[i];
            QString date = pair.first;
            QDateTime time = QDateTime {{QDate::fromString(date, "yyyy.MM")}};
            qreal x = (qreal) time.toMSecsSinceEpoch();;
            qreal y = pair.second;
            series->append(i, y);  //  Добавление срезов к серии
        }
        chartView->chart()->addSeries(series);
        //chartView->chart()->legend()->show();
        chartView->chart()->setTitle("Линейная диаграмма");
        chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);
        chartView->setRenderHint(QPainter::Antialiasing);
    }
};


#endif // DATAPLOTTER_H
