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
        PrepareData(chartView, data);           //  Первый шаг - подготовка полученных данных (здесь закладываем их в chartView)
        ConfigureChart(chartView);              //  Второй шаг - настройка диаграммы (в зависимости от типа)
        chartView->update();                    //  Обновление окна вывода
    };

protected:
    virtual void PrepareData(QChartView* chartView, const QList<QPair<QString, qreal>>& data) = 0;  //  Сами абстрактные операции,
    virtual void ConfigureChart(QChartView* chartView) = 0;                                         //  определяемые в наследниках АК
};


class BarDataPlotter : public DataPlotter  //  Класс для создания столбчатых диаграмм
{
protected:
    void PrepareData(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
    {
        qreal minValue = std::numeric_limits<qreal>::max();
        qreal maxValue = std::numeric_limits<qreal>::lowest();
        QBarSeries *series = new QBarSeries();     //   Создание серии пустых столбцов
        for (int i = 0; i < 10; ++i)
        {
            const QPair<QString, qreal>& pair = data[i];
            QString time = pair.first;
            qreal value = pair.second;
            QBarSet *barSet = new QBarSet(time);   //  Создание набора данных с меткой time.
            *barSet << value;                      //  Добавление данных к набору с помощью оператора <<.
            series->append(barSet);                //  Добавление наборов данных к серии
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
        }
        chartView->chart()->addSeries(series);     //  Добавление серии к диаграмме
    }
    void ConfigureChart(QChartView* chartView)
    {
        chartView->chart()->setTitle("Столбчатая диаграмма");               //  Определение заголовка диаграммы.
        chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);  //  Определение параметров анимации для диаграммы (Включена анимация серий).
        chartView->setRenderHint(QPainter::Antialiasing);                   //  Установка данный флага на художника
    }                                                                       //  (Движок должен сглаживать края примитивов, если это возможно).
};


class PieDataPlotter : public DataPlotter  //  Класс для создания круговых диаграмм
{
protected:
    void PrepareData(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
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
    }
    void ConfigureChart(QChartView* chartView)
    {
        chartView->chart()->setTitle("Круговая диаграмма");
        chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);
        chartView->setRenderHint(QPainter::Antialiasing);
    }
};


#endif // DATAPLOTTER_H
