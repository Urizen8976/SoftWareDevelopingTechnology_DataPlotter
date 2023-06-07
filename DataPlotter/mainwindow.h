#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtWidgets>           //  Подключение модуля классов для расширения Qt GUI виджетами C++.


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

#include <QtSql>                //  Подключение библиотеки для работы с БД
#include <QSqlQueryModel>

#include <QtCharts/QChartView>  //  Подключение библиотеки для работы с графиками
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts>
#include <QPainter>
#include <QPdfWriter>
/*
#include "ioc_container.h"
#include "chartdrawer.h"*/


QT_BEGIN_NAMESPACE
namespace Ui {  class MainWindow;  }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setChartStrategy(std::shared_ptr<ChartStrategy> strategy) {   chartStrategy = strategy;   }
    void drawChart() {   chartStrategy->draw(chartView, fileData);   }

private:
    Ui::MainWindow *ui;

    QFileSystemModel *rightPartModel;              //  Файловая система
    QFileSystemModel *leftPartModel;
    QTreeView *treeView;
    QListView *listView;
    QChartView *chartView;                         //  Все для графика
    //    QChart *chart;
    //    QBarCategoryAxis *axisX;
    //    QValueAxis *axisY;
    QComboBox *comboBox;                           //  Все для комбобокса
    QString filePath;                              //  Путь до файла
    QList<QPair<QString, qreal>> fileData;         //  Данные файла
    QCheckBox *checkBox;                           //  Выбор цвета
    QPushButton* openTreeView;                     //  Кнопка открытия дерева файлов
    QLabel* diagrammType;                          //  Текст "Выбрать тип диаграммы"
    IOCContainer DataGetterContainer;              //  IoC-контейнер
    std::shared_ptr<ChartStrategy> chartStrategy;  //  Стратегия для рисования графика

private slots:
    void on_selectionTreeChangedSlot(const QItemSelection &selected, const QItemSelection &deselected);
    void on_selectionListChangedSlot(const QItemSelection &selected, const QItemSelection &deselected);
    void comboBoxItemSelected(int index);
    void onCheckBoxStateChanged(int state);
    void onButtonOpenTreeView();
};
#endif // MAINWINDOW_H
