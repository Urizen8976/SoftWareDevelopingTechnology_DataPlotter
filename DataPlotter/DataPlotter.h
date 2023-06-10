#ifndef DATAPLOTTER_H
#define DATAPLOTTER_H


#include <QChartView>   // Подключаем библиотеки для работы с графиками
#include <QBarSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts>
#include <QPainter>
#include <QPdfWriter>


class DataPlotter
{
public:
    void DrawChart(QChartView* chartView, const QList<QPair<QString, qreal>>& data)  //  Общий реализуемый алгоритм
    {
        chartView->chart()->removeAllSeries();  //  Очистка окна рисования
        PrepareData(chartView, data);           //  Первый шаг - подготовка полученных данных (здесь закладываем их в chartView)
        ConfigureChart(chartView);              //  Второй шаг - настройка диаграммы (в зависимости от типа)
        chartView->update();                    //  Обновление окна вывода
    };

protected:
    // Данные функции должны быть переопределены наследниками в зависимости от их предпочтений
    virtual void PrepareData(QChartView* chartView, const QList<QPair<QString, qreal>>& data) = 0;
    virtual void ConfigureChart(QChartView* chartView) = 0;
};


class BarDataPlotter : public DataPlotter  //  Класс для создания столбчатых диаграмм
{
protected:
    void PrepareData(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
    {
        qreal minValue = std::numeric_limits<qreal>::max();
        qreal maxValue = std::numeric_limits<qreal>::lowest();
        QBarSeries *series = new QBarSeries();
        for (int i = 0; i < 10; ++i)
        {
            const QPair<QString, qreal>& pair = data[i];
            QString time = pair.first;
            qreal value = pair.second;
            QBarSet *barSet = new QBarSet(time);
            *barSet << value;
            series->append(barSet);
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
        }
        chartView->chart()->addSeries(series);
    }
    void ConfigureChart(QChartView* chartView)
    {
        chartView->chart()->setTitle("Столбчатая диаграмма");
        chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);
        chartView->setRenderHint(QPainter::Antialiasing);
    }
};


class PieDataPlotter : public DataPlotter  //  Класс для создания круговых диаграмм
{
protected:
    void PrepareData(QChartView* chartView, const QList<QPair<QString, qreal>>& data)
    {
        QPieSeries *series = new QPieSeries();
        for (int i = 0; i < 10; ++i)
        {
            const QPair<QString, qreal>& pair = data[i];
            QString time = pair.first;
            qreal value = pair.second;
            series->append(time, value);
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
