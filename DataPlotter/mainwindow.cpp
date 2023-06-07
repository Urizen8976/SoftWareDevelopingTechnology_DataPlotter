#include "MainWindow.h"
#include "ui_MainWindow.h"


#include "ioc_container.h"
#include "datagetter.h"
#include "chartdrawer.h"


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
    formats << "sqlite" << "json" << "csv";
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

    // Установка осей
//    axisX = new QBarCategoryAxis();
//    chart->addAxis(axisX, Qt::AlignBottom);
//    axisY = new QValueAxis();
//    chart->addAxis(axisY, Qt::AlignLeft);

    comboBox = new QComboBox();                            //  Настройка выбора графиков
    comboBox->addItem("Столбчатая диаграмма");
    comboBox->addItem("Круговая диаграмма");
    checkBox = new QCheckBox("Черно-белый");               //  Добавление выбора цвета - checkBox
    openTreeView = new QPushButton("Открыть");             //  Настройка кнопки для открытия дерева выбора папки
    diagrammType = new QLabel("Выберите тип диаграммы");   //  Настройка строки "Выберите тип диаграммы"

    //---------------------------------------------------------------------------------------------------
    // Компоновка окна
    // Создание главного макета
    // Часть окна с функциями и часть окна с выводом
    QHBoxLayout *functionLayout = new QHBoxLayout();
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


    QItemSelectionModel *treeSelectionModel = treeView->selectionModel();  // Отслеживание выбранного файла в дереве
    connect(treeSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::on_selectionTreeChangedSlot);
    QItemSelectionModel *listSelectionModel = listView->selectionModel();  // Отслеживание выбранного файла в таблице файлов
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


//  Слот для обработки выбора элемента в TreeView. Выбор осуществляется с помощью курсора.
void MainWindow::on_selectionTreeChangedSlot(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    //QModelIndex index = treeView->selectionModel()->currentIndex();
    QModelIndexList indexs =  selected.indexes();
    QString filePath = "";

    if (indexs.count() >= 1) {                              //  Определение количества выделенных индексов.
        QModelIndex ix =  indexs.constFirst();              //  Выделение только одного (В нашем случае), => всегда берем только первый.
        filePath = leftPartModel->filePath(ix);
        this->statusBar()->showMessage(                     //  Размещение информации в statusbar относительно выделенного модельного индекса
            "Выбранный путь : " + leftPartModel->filePath(indexs.constFirst()));
    }
    // Получив выбранные данные из левой части filePath(путь к папке/файлу).
    // Для представления в правой части устанваливаем корневой индекс относительно filePath.
    // Табличное представление отображает только файлы, находящиеся в filePath (папки не отображает)
    listView->setRootIndex(rightPartModel->setRootPath(filePath));
}


// Слот для обработки выбора элемента в TableView. Добавить проверку новых данных перед рисованием
void MainWindow::on_selectionListChangedSlot(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    QModelIndex index = listView->selectionModel()->currentIndex();
    filePath = rightPartModel->filePath(index);
    this->statusBar()->showMessage("Выбранный файл : " + filePath);

    DataGetterContainer.RegisterInstance<IDataGetter, SQLiteDataGetter>();

    if(DataGetterContainer.GetObject<IDataGetter>()->CheckFile(filePath))
    {
        fileData = DataGetterContainer.GetObject<IDataGetter>()->getData(filePath);
        if(!fileData.isEmpty())
        {
            QString selectedText = comboBox->currentText();
            if (selectedText == "Столбчатая диаграмма")
            {
                DataGetterContainer.RegisterInstance<ChartStrategy, BarChartStrategy>();
                setChartStrategy(DataGetterContainer.GetObject<ChartStrategy>());
                drawChart();
            }
            if (selectedText == "Круговая диаграмма")
            {
                DataGetterContainer.RegisterInstance<ChartStrategy, PieChartStrategy>();
                setChartStrategy(DataGetterContainer.GetObject<ChartStrategy>());
                drawChart();
            }
        }
        else
        {
        //вывод отсутствия данных на экран
        }
    }
}


void MainWindow::onCheckBoxStateChanged(int state)
{
    if (state == Qt::Checked)
    {
        QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect;
        effect->setColor(Qt::black);
        chartView->chart()->setGraphicsEffect(effect);
    }
    else {   chartView->chart()->setGraphicsEffect(nullptr);   }
}


void MainWindow::onButtonOpenTreeView() {   treeView -> show();   }


void MainWindow::comboBoxItemSelected(int index)
{
    if(!fileData.isEmpty())
    {
            QString selectedText = comboBox->currentText();
            if (selectedText == "Столбчатая диаграмма")
            {
                DataGetterContainer.RegisterInstance<ChartStrategy, BarChartStrategy>();
                setChartStrategy(DataGetterContainer.GetObject<ChartStrategy>());
                drawChart();
            }
            if (selectedText == "Круговая диаграмма")
            {
                DataGetterContainer.RegisterInstance<ChartStrategy, PieChartStrategy>();
                setChartStrategy(DataGetterContainer.GetObject<ChartStrategy>());
                drawChart();
            }
    }
    else
    {}   //вывод отсутствия данных на экран
}


MainWindow::~MainWindow() {   delete ui;   }
