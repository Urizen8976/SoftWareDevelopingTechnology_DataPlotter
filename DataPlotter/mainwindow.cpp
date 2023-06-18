#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "IOC-Container.h"
#include "DataGetter.h"
#include "DataPlotter.h"


int IOCContainer::s_nextTypeId = 0;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);                                                  //  Первоначальная настройка главного окна
    this->setGeometry(100, 100, 1500, 500);                             //  Установка размера главного окна
    this->setStatusBar(new QStatusBar(this));                           //  Установка строки состояния (гори-ую полосу с информацией) для окна .
    this->statusBar()->showMessage("Выбранный путь : ");

    QString homePath = QDir::homePath();                                //  Настройка файловых систем
    leftPartModel = new QFileSystemModel(this);                         //  Первоначальная настройка дерева файлов (файловой системы)
    leftPartModel->setFilter(                                           //  Установка фильтра модели каталога
        QDir::AllDirs |                                                 //  (Чтобы был виден список всех каталогов
        QDir::NoDotAndDotDot | QDir::NoSymLinks);                       //  без специальных записей "." и "..", и символических ссылок)
    leftPartModel->setRootPath(homePath);                               //  Установка каталога, за которым следит модель

    rightPartModel = new QFileSystemModel(this);
    rightPartModel->setFilter(
        QDir::Files |                                                   //  (Чтобы был виден cписок файлов)
        QDir::NoDotAndDotDot | QDir::NoSymLinks);
    rightPartModel->setRootPath(homePath);

    QStringList formats;
    formats << "sqlite" << "json" << "csv";                //  Добавление строк оператором <<
    QStringList filters;
    for (const QString& format : formats)
    {
        filters.append(QString("*.%1").arg(format));       //  Фильтрация типов данных для листа файлов по условию задачи
        //  Ну или можно реализовать через конкатенацию строк
    }                                                      //  С заменой "%1" на "format" в строке
    rightPartModel->setNameFilters(filters);               //  Установка имени filters для применения к существующим файлам.
    rightPartModel->setNameFilterDisables(false);          //  Это свойство определяет, скрыты или отключены файлы, не прошедшие фильтр имен.

    treeView = new QTreeView();                            //  Настройка дерева файлов на основе leftPartModel через готовый вид(TreeView):
    treeView->setModel(leftPartModel);                     //  Установка модели данных для отображения
    treeView->expandAll();                                 //  (Публичный слот) Раскрытие все папок первого уровня
    treeView->header()->resizeSection(0, 200);             //  Изменение размера раздела
    listView = new QListView();                            //  Настройка таблицы файлов на основе rightPartModel
    listView->setModel(rightPartModel);
    chartView = new QChartView();                          //  Первоначальная настройка графика
    chartView->setRenderHint(QPainter::Antialiasing);      //  Движок должен сглаживать края примитивов, если это возможно.
    chartView->setMinimumSize(600, 400);

    comboBox = new QComboBox();                            //  Настройка выбора графиков (через комбинированную кнопку со всплывающим списком)
    comboBox->addItem("Столбчатая диаграмма");
    comboBox->addItem("Круговая диаграмма");
    comboBox->addItem("Линейная диаграмма");
    checkBox = new QCheckBox("Черно-белый");               //  Добавление выбора цвета
    //checkBox = new QComboBox();
    //checkBox->addItem("Черно-белый");
    openTreeView = new QPushButton("Открыть");             //  Настройка кнопки для открытия дерева выбора папки
    saveToPDF = new QPushButton("Сохранить в PDF");

    QHBoxLayout *functionLayout = new QHBoxLayout();       //  Создание верхнего макета
    QSplitter *splitter = new QSplitter(parent);           //  Создание серединного виджета-разделителя
    QHBoxLayout *functionLayout2 = new QHBoxLayout();      //  Создание нижнего макета
    functionLayout->addWidget(openTreeView);               //  Добавление виджетов на часть с функциями
    functionLayout2->addWidget(comboBox);
    functionLayout2->addWidget(checkBox);
    functionLayout2->addWidget(saveToPDF);                  //
    splitter->addWidget(listView);                         //  Добавление виджетов на QSplitter
    splitter->addWidget(chartView);

    QVBoxLayout *mainLayout = new QVBoxLayout();           //  Основное окно
    mainLayout->addLayout(functionLayout);                 //  Добавление QSplitter и виджета с графиком на главный макет
    mainLayout->addWidget(splitter);
    mainLayout->addLayout(functionLayout2);
    QWidget *mainWidget = new QWidget();                   //  Создание виджета для главного макета
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);                          //  Установка виджета на главное окно

    QItemSelectionModel *treeSelectionModel = treeView->selectionModel();  //  Создание модели выбора для отслеживания элементов в представлении
    connect(treeSelectionModel, &QItemSelectionModel::selectionChanged,    //  Сигнал, испускающийся при изменении выбора
        this, &MainWindow::on_selectionTreeChangedSlot);                   //  Слот обработки изменения выбора в TreeView.
    QItemSelectionModel *listSelectionModel = listView->selectionModel();  //  Отслеживание выбранного файла в таблице файлов
    connect(listSelectionModel, &QItemSelectionModel::selectionChanged,
        this, &MainWindow::on_selectionListChangedSlot);                   //  Слот обработки изменения выбора TreeView.
    connect(comboBox, SIGNAL(activated(int)),                              //  Соединение для изменения вида графика ("Столбчатая диаграмма")
        this, SLOT(comboBoxItemSelected()));
    connect(checkBox, &QCheckBox::stateChanged,                            //  Соединение для изменения цветов графика ("Ч-Б")
        this, &MainWindow::onCheckBoxStateChanged);
    connect(openTreeView, &QPushButton::clicked,                           //  Соединение сигнала от кнопки ("открыть")
        this, &MainWindow::onButtonOpenTreeView);
    connect(saveToPDF, &QPushButton::clicked,
        this, &MainWindow::onButtonSaveToPDFPressed);

    QItemSelection toggleSelection;                        //  Пример организации установки курсора в TreeView отн-но модельного индекса
    QModelIndex topLeft;                                   //  Объявление модельного индекса topLeft
    topLeft = leftPartModel->index(homePath);              //  Получение индекса из модели
    toggleSelection.select(topLeft, topLeft);              //  Выбор элемента модели
    treeSelectionModel->select(toggleSelection,
        QItemSelectionModel::Toggle);                      //  Все указанные индексы будут выбраны или отменены в зав-ти от их текущего состояния.
}


void MainWindow::on_selectionTreeChangedSlot(  //  Слот для обработки выбора элемента в TreeView. Выбор осуществляется с помощью курсора.
const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    QModelIndexList indexs =  selected.indexes();
    QString filePath = "";
    if (indexs.count() >= 1) {                              //  Определение количества выделенных индексов.
        QModelIndex ix =  indexs.constFirst();              //  Выделение только одного (В нашем случае), => всегда берем только первый.
        filePath = leftPartModel->filePath(ix);
        this->statusBar()->showMessage(                     //  Размещение информации в statusbar относительно выделенного модельного индекса
            "Выбранный путь : " + leftPartModel->filePath(indexs.constFirst()));
    }
    listView->setRootIndex(rightPartModel->setRootPath(filePath));  //  Установка корневого индекса относительно filePath (из левой части)
}


void MainWindow::on_selectionListChangedSlot(  //  Слот для обработки выбора элемента в ListView. Выбор осуществляется с помощью курсора.
const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    Q_UNUSED(selected);
    QModelIndex index = listView->selectionModel()->currentIndex();
    filePath = rightPartModel->filePath(index);
    this->statusBar()->showMessage("Выбранный файл : " + filePath);
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix();
    qDebug() << "Расширение файла:" << extension;
    if (extension == "json")
    {
        m_container.RegisterInstance<IStrategy, JSONStrategy>();
        SetStrategy(m_container.GetObject<IStrategy>());
    }
    else if (extension == "sqlite")
    {
        m_container.RegisterInstance<IStrategy, SQLiteStrategy>();
        SetStrategy(m_container.GetObject<IStrategy>());
    }
    /*else if (extension == "csv")
    {
        m_container.RegisterInstance<IStrategy, CSVStrategy>();
        SetStrategy(m_container.GetObject<IStrategy>());
    }*/
    if(CheckFile())
    {
        qDebug() << "Проверка файла пройдена успешно.";
        fileData = GetData();
        /*for (int i = 0; i < qMin(10, fileData.size()); ++i)
        {
            const QPair<QString, qreal>& value = fileData[i];
            qDebug() << "Time:" << value.first << "Value:" << value.second;
        }*/
        if(!fileData.isEmpty())
        {
            QString selectedText = comboBox->currentText();
            if (selectedText == "Столбчатая диаграмма")
            {
                m_container.RegisterInstance<DataPlotter, BarDataPlotter>();
                m_container.GetObject<DataPlotter>()->DrawChart(chartView, fileData);
            }
            if (selectedText == "Круговая диаграмма")
            {
                m_container.RegisterInstance<DataPlotter, PieDataPlotter>();
                m_container.GetObject<DataPlotter>()->DrawChart(chartView, fileData);
            }
            if (selectedText == "Линейная диаграмма")
            {
                m_container.RegisterInstance<DataPlotter, LineDataPlotter>();
                m_container.GetObject<DataPlotter>()->DrawChart(chartView, fileData);
            }
        }
        else{}
    }
}


void MainWindow::onCheckBoxStateChanged(int state)       //  Слот для обработки нажатия Чек-бокса: изменяем эффект на графике
{
    if (state == Qt::Checked)                            //  Проверка состояния для применения эффекта
    {
        QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect;
        effect->setColor(Qt::black);
        chartView->chart()->setGraphicsEffect(effect);
    }
    else
    {
        chartView->chart()->setGraphicsEffect(nullptr);  //  Проверка состояния для применения эффекта
    }
}


void MainWindow::onButtonOpenTreeView()  //  Слот для обработки нажатия кнопки "Открыть" : кнопка открытия дерева
{
    treeView->resize(600, 600);          //  Подредактируем размер
    if (treeView->isVisible())           //  Если окно уже отображается (открыто)
    {   treeView->raise();   }           //  Выведем его поверх
    else
    {   treeView->show();   }            //  Иначе отобразим его
}


void MainWindow::comboBoxItemSelected()
{
    if(!fileData.isEmpty())
    {
            QString selectedText = comboBox->currentText();
            if (selectedText == "Столбчатая диаграмма")
            {
                m_container.RegisterInstance<DataPlotter, BarDataPlotter>();
                m_container.GetObject<DataPlotter>()->DrawChart(chartView, fileData);
            }
            if (selectedText == "Круговая диаграмма")
            {
                m_container.RegisterInstance<DataPlotter, PieDataPlotter>();
                m_container.GetObject<DataPlotter>()->DrawChart(chartView, fileData);
            }
            if (selectedText == "Линейная диаграмма")
            {
                m_container.RegisterInstance<DataPlotter, LineDataPlotter>();
                m_container.GetObject<DataPlotter>()->DrawChart(chartView, fileData);
            }
    }
    else {}
}


void MainWindow::onButtonSaveToPDFPressed()
{
    auto filePath = QFileDialog::getSaveFileName(nullptr, "Save chart as a PDF document", {}, "PDF (*.pdf)"); //  Диалоговое окно, позволяющее
    if (filePath.isEmpty()) return;                                                                           //  выбирать файлы или каталоги
    QPdfWriter pdfWriter = QPdfWriter {filePath};
    QPainter painter = QPainter {&pdfWriter};
    chartView->render(&painter);
    painter.end();
}


MainWindow::~MainWindow() {   delete ui;   }
