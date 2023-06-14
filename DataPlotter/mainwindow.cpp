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
    this->setStatusBar(new QStatusBar(this));
    this->statusBar()->showMessage("Выбранный путь : ");

    QString homePath = QDir::homePath();                                //  Настройка файловых систем
    leftPartModel = new QFileSystemModel(this);                         //  Первоначальная настройка дерева файлов (файловой системы)
    leftPartModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    leftPartModel->setRootPath(homePath);

    rightPartModel = new QFileSystemModel(this);                        //  Первоначальная настройка таблицы файлов (файловой системы)
    rightPartModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    rightPartModel->setRootPath(homePath);

    QStringList formats;
    formats << "sqlite" << "json" << "csv";                //  Фильтрация типов данных для листа файлов по условию задачи
    QStringList filters;
    for (const QString& format : formats) {
        filters.append(QString("*.%1").arg(format));
    }
    rightPartModel->setNameFilters(filters);
    rightPartModel->setNameFilterDisables(false);

    treeView = new QTreeView();                            //  Настройка дерева файлов на основе leftPartModel через готовый вид(TreeView):
    treeView->setModel(leftPartModel);                     //  Установка модели данных для отображения
    treeView->expandAll();                                 //  Раскрытие все папок первого уровня
    treeView->header()->resizeSection(0, 200);

    listView = new QListView();                            //  Настройка таблицы файлов на основе rightPartModel
    listView->setModel(rightPartModel);

    chartView = new QChartView();                          //  Первоначальная настройка графика
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(600, 400);

    comboBox = new QComboBox();                            //  Настройка выбора графиков
    comboBox->addItem("Столбчатая диаграмма");
    comboBox->addItem("Круговая диаграмма");
    comboBox->addItem("Линейная диаграмма");
    checkBox = new QCheckBox("Черно-белый");               //  Добавление выбора цвета - checkBox
    openTreeView = new QPushButton("Открыть");             //  Настройка кнопки для открытия дерева выбора папки
    diagrammType = new QLabel("Выберите тип диаграммы");   //  Настройка строки "Выберите тип диаграммы"

    QHBoxLayout *functionLayout = new QHBoxLayout();       //  Создание главного макета
    QSplitter *splitter = new QSplitter(parent);           //  Создание объекта "сплиттер(разделитель)"

    functionLayout->addWidget(openTreeView);               //  Добавление виджетов на часть с функциями
    functionLayout->addWidget(diagrammType);
    functionLayout->addWidget(comboBox);
    functionLayout->addWidget(checkBox);
    splitter->addWidget(listView);                         //  Добавление виджетов на QSplitter
    splitter->addWidget(chartView);

    QVBoxLayout *mainLayout = new QVBoxLayout();           //  Основное окно
    mainLayout->addLayout(functionLayout);                 //  Добавление QSplitter и виджета с графиком на главный макет
    mainLayout->addWidget(splitter);

    QWidget *mainWidget = new QWidget();                   //  Создание виджета для главного макета
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);                          //  Установка виджета на главное окно


    QItemSelectionModel *treeSelectionModel = treeView->selectionModel();                   // Отслеживание выбранного файла в дереве
    connect(treeSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::on_selectionTreeChangedSlot);
    QItemSelectionModel *listSelectionModel = listView->selectionModel();                   // Отслеживание выбранного файла в таблице файлов
    connect(listSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::on_selectionListChangedSlot);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(comboBoxItemSelected(int)));
    connect(checkBox, &QCheckBox::stateChanged, this, &MainWindow::onCheckBoxStateChanged); // Соединение для изменения цветов графика
    connect(openTreeView, &QPushButton::clicked, this, &MainWindow::onButtonOpenTreeView);  // Соединение сигнала от кнопки

    QItemSelection toggleSelection;                        //  Пример организации установки курсора в TreeView отн-но модельного индекса
    QModelIndex topLeft;                                   //  Объявление модельного индекса topLeft
    topLeft = leftPartModel->index(homePath);              //  Получение индекса из модели
    toggleSelection.select(topLeft, topLeft);
    treeSelectionModel->select(toggleSelection, QItemSelectionModel::Toggle);
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


void MainWindow::on_selectionListChangedSlot(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    Q_UNUSED(selected);
    QModelIndex index = listView->selectionModel()->currentIndex();
    filePath = rightPartModel->filePath(index);
    this->statusBar()->showMessage("Выбранный файл : " + filePath);
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix();
    qDebug() << "Расширение файла:" << extension;
    // Пока что установка стратегии реализована так
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
    if(CheckFile())
    {
        // отладочная информация
        qDebug() << "Проверка файла пройдена успешно.";
        fileData = GetData();

        for (int i = 0; i < qMin(10, fileData.size()); ++i)
        {
            const QPair<QString, qreal>& value = fileData[i];
            qDebug() << "Time:" << value.first << "Value:" << value.second;
        }

        // Пока что выбор графика для рисования реализован так
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
        }
        else
        {
        //вывод отсутствия данных на экран
        }
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
    }
    else
    {}   //вывод отсутствия данных на экран
}


MainWindow::~MainWindow() {   delete ui;   }
