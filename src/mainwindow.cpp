#include "mainwindow.h"
#include "codeeditor.h"
#include "newdialog.h"
#include "tabwidget.h"
#include "finddialog.h"
#include "newtablewidgett.h"
#include "highlighter.h"
#include "mydockwidget.h"
#include "exportwizard.h"
#include "importwizard.h"
#include <QApplication>
#include <QAction>
#include <QMenuBar>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QSettings>
#include <QCloseEvent>
#include <QDockWidget>
#include <QTreeView>
#include <QInputDialog>
#include <QUrl>
#include <QSqlIndex>
#include <QMap>
#include <QStringList>
#include <QDebug>

bool MainWindow::isReadOnlyMenu = true;
bool MainWindow::isShowMenu = true;
bool MainWindow::isAssociate = true;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    findDialog = 0;

    dockWidget = new MyDockWidget(tr("Database File"), this);
    treeWidget = new QTreeWidget;
    treeWidget->setHeaderLabels(QStringList() << tr("Database"));
    dockWidget->setWidget(treeWidget);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea
                                | Qt::RightDockWidgetArea);
    dockWidget->setObjectName("fileDockWidget");

    tabWidget = new TabWidget;

    rightSplitter = new QSplitter(Qt::Vertical);
    rightSplitter->addWidget(tabWidget);
    rightSplitter->setHandleWidth(10);

    setCentralWidget(rightSplitter);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    setWindowIcon(QIcon(":/images/icon.png"));
    setWindowTitle(tr("SQLite Database Manage"));

    connect(treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(treeItemDoubleClicked(QTreeWidgetItem*,int)));
    connect(dockWidget, SIGNAL(dockWidgetShow(bool)), showDockWidgetAction, SLOT(setChecked(bool)));

    connect(tabWidget, SIGNAL(widgetChanged(WidgetType)), this, SLOT(tabWidgetChanged(WidgetType)));

    readSettings();

    setAcceptDrops(true);
    setActionPrivate(false, false);
    createNewTableAction->setEnabled(false);
    createNewSQLAction->setEnabled(false);
    saveAction->setEnabled(false);
    saveAllAction->setEnabled(false);
    setTableAction(false);
    setTableIndexAction(false, false);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
    newAction = new QAction(tr("&New Database"), this);
    newAction->setIcon(QIcon(":/images/new.png"));
    newAction->setShortcut(tr("Ctrl+N"));
    newAction->setStatusTip(tr("Create a new Database file"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(newDatabase()));

    openAction = new QAction(tr("&Open Database"), this);
    openAction->setIcon(QIcon(":/images/open.png"));
    openAction->setShortcut(tr("Ctrl+O"));
    openAction->setStatusTip(tr("Open an existing Database file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(openDatabaseFile()));

    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        recentFileAction[i] = new QAction(this);
        recentFileAction[i]->setVisible(false);
        connect(recentFileAction[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }

    saveAction = new QAction(tr("&Save"), this);
    saveAction->setIcon(QIcon(":/images/save.png"));
    saveAction->setShortcut(tr("Ctrl+S"));
    saveAction->setStatusTip(tr("Save the Database to disk"));
    connect(saveAction, SIGNAL(triggered()), tabWidget, SLOT(save()));

    saveAllAction = new QAction(tr("Save &All"), this);
    saveAllAction->setIcon(QIcon(":/images/saveall.png"));
    saveAllAction->setStatusTip(tr("Save the All Table Modify to disk"));
    connect(saveAllAction, SIGNAL(triggered()), tabWidget, SLOT(saveAll()));

    saveSqlAction = new QAction(tr("Save SQL as"), this);
    saveSqlAction->setIcon(QIcon(":/images/sqlfilesave.png"));
    saveSqlAction->setShortcut(tr("Ctrl+Shift+S"));
    saveSqlAction->setStatusTip(tr("Save the SQL to disk"));
    connect(saveSqlAction, SIGNAL(triggered()), tabWidget, SLOT(saveSqlAs()));

    openSqlAction = new QAction(tr("Load SQL File"), this);
    openSqlAction->setIcon(QIcon(":/images/sqlfileopen.png"));
    openSqlAction->setStatusTip(tr("Load the SQL File under disk"));
    connect(openSqlAction, SIGNAL(triggered()), this, SLOT(openSql()));

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setIcon(QIcon(":/images/exit.png"));
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    cutAction = new QAction(tr("Cu&t"), this);
    cutAction->setIcon(QIcon(":/images/cut.png"));
    cutAction->setShortcut(tr("Ctrl+X"));
    cutAction->setStatusTip(tr("Cut the current selection's contents "
                               "to the clipboard"));
    connect(cutAction, SIGNAL(triggered()), tabWidget, SLOT(cut()));

    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setIcon(QIcon(":/images/copy.png"));
    copyAction->setShortcut(tr("Ctrl+C"));
    copyAction->setStatusTip(tr("Copy the current selection's contents "
                                "to the clipboard"));
    connect(copyAction, SIGNAL(triggered()), tabWidget, SLOT(copy()));

    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setIcon(QIcon(":/images/paste.png"));
    pasteAction->setShortcut(tr("Ctrl+V"));
    pasteAction->setStatusTip(tr("Paste the clipboard's contents into "
                                 "the current selection"));
    connect(pasteAction, SIGNAL(triggered()), tabWidget, SLOT(paste()));

    deleteAction = new QAction(tr("&Delete"), this);
    deleteAction->setIcon(QIcon(":/images/delete.png"));
    deleteAction->setShortcut(tr("Del"));
    deleteAction->setStatusTip(tr("Delete the current selection's "
                                  "contents"));
    connect(deleteAction, SIGNAL(triggered()), tabWidget, SLOT(del()));

    insertRowAction = new QAction(tr("&Insert row"), this);
    insertRowAction->setIcon(QIcon(":/images/insertrow.png"));
    insertRowAction->setStatusTip(tr("Insert a row in current Table"));
    connect(insertRowAction, SIGNAL(triggered()), tabWidget, SLOT(insertRow()));

    deleteRowAction = new QAction(tr("&Delete rows"), this);
    deleteRowAction->setIcon(QIcon(":/images/deleterow.png"));
    deleteRowAction->setStatusTip(tr("Delete the current selection's rows"));
    connect(deleteRowAction, SIGNAL(triggered()), tabWidget, SLOT(delRow()));

    findAction = new QAction(tr("&Find"), this);
    findAction->setIcon(QIcon(":/images/find.png"));
    findAction->setShortcut(tr("Ctrl+F"));
    findAction->setStatusTip(tr("Find a matching cell"));
    connect(findAction, SIGNAL(triggered()), this, SLOT(find()));


    connect(treeWidget, SIGNAL(itemPressed(QTreeWidgetItem*,int)),
            this, SLOT(slotShowTreeContextMenu(QTreeWidgetItem*,int)));

    closeConnectAction = new QAction(tr("&Close connect"), this);
    closeConnectAction->setIcon(QIcon(":/images/closeconnect.png"));
    closeConnectAction->setStatusTip(tr("Close current Database connect"));
    connect(closeConnectAction, SIGNAL(triggered()), this, SLOT(closeDatabaseConnect()));

    analysisSqlAction = new QAction(tr("Analysis SQL"), this);
    analysisSqlAction->setIcon(QIcon(":/images/isnull.png"));
    analysisSqlAction->setStatusTip(tr("Analysis SQL Statement"));
    connect(analysisSqlAction, SIGNAL(triggered()), tabWidget, SLOT(analysisSql()));

    runLineSqlAction = new QAction(tr("Run Select SQL"), this);
    runLineSqlAction->setIcon(QIcon(":images/runsql.png"));
    runLineSqlAction->setStatusTip(tr("Execute Select SQL"));
    connect(runLineSqlAction, SIGNAL(triggered()), tabWidget, SLOT(runLineSql()));

    runSqlAction = new QAction(tr("Run SQL"), this);
    runSqlAction->setShortcut(tr("F9"));
    runSqlAction->setIcon(QIcon(":/images/run.png"));
    runSqlAction->setStatusTip(tr("Execute SQL"));
    connect(runSqlAction, SIGNAL(triggered()), tabWidget, SLOT(onSqlExecute()));

    showGridAction = new QAction(tr("&Show Grid"), this);
    showGridAction->setCheckable(true);
    showGridAction->setChecked(true);
    showGridAction->setStatusTip(tr("Show or hide the Table "
                                    "grid"));
    connect(showGridAction, SIGNAL(triggered(bool)),
            this, SLOT(setShowGrid(bool)));

    readOnlyAction = new QAction(tr("&Read Only"), this);
    readOnlyAction->setCheckable(true);
    readOnlyAction->setChecked(true);
    readOnlyAction->setStatusTip(tr("Set the Table only read"));
    connect(readOnlyAction, SIGNAL(triggered(bool)), this, SLOT(setReadOnly(bool)));

    showDockWidgetAction = new QAction(tr("&Show File Dialog"), this);
    showDockWidgetAction->setCheckable(true);
    showDockWidgetAction->setChecked(true);
    showDockWidgetAction->setStatusTip(tr("Show or hide the File Dialog"));
    connect(showDockWidgetAction, SIGNAL(triggered(bool)), this, SLOT(showDockWidget(bool)));

    associateAction = new QAction(tr("Associate"), this);
    associateAction->setCheckable(true);
    associateAction->setChecked(true);
    associateAction->setStatusTip(tr("Open or Close the Associate"));
    connect(associateAction, SIGNAL(triggered(bool)), this, SLOT(Associate(bool)));

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));



    createNewTableAction = new QAction(tr("Create New Table"), this);
    createNewTableAction->setIcon(QIcon(":/images/tableadd.png"));
    createNewTableAction->setStatusTip(tr("Create a new table"));
    connect(createNewTableAction, SIGNAL(triggered()), this, SLOT(createNewTable()));

    refreshAction = new QAction(tr("Refresh Data"), this);
    refreshAction->setIcon(QIcon(":/images/refresh.png"));
    refreshAction->setStatusTip(tr("Refresh Data"));
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(refreshData()));

    openTableAction = new QAction(tr("Open the table"), this);
    openTableAction->setIcon(QIcon(":/images/opentable.png"));
    openTableAction->setStatusTip(tr("Open the table"));
    connect(openTableAction, SIGNAL(triggered()), this, SLOT(openNewTable()));

    renameTableAction = new QAction(tr("Rename the Table"), this);
    renameTableAction->setIcon(QIcon(":/images/renametable.png"));
    renameTableAction->setStatusTip(tr("Rename the Table"));
    connect(renameTableAction, SIGNAL(triggered()), this, SLOT(renameTheTable()));

    deleteTableAction = new QAction(tr("Delete table"), this);
    deleteTableAction->setIcon(QIcon(":/images/tabledelete.png"));
    deleteTableAction->setStatusTip(tr("Delete a table"));
    connect(deleteTableAction, SIGNAL(triggered()), this, SLOT(deleteTable()));

    modifyTableFieldAction = new QAction(tr("Modify Table Field"), this);
    modifyTableFieldAction->setIcon(QIcon(":/images/tableedit.png"));
    modifyTableFieldAction->setStatusTip(tr("Modify Table Field"));
    connect(modifyTableFieldAction, SIGNAL(triggered()), this, SLOT(modifyTableField()));

    createNewSQLAction = new QAction(tr("Create New SQL"), this);
    createNewSQLAction->setIcon(QIcon(":/images/newsqlfile.png"));
    createNewSQLAction->setStatusTip(tr("Create New Query on Table"));
    connect(createNewSQLAction, SIGNAL(triggered()), this, SLOT(createNewQuery()));

    createNewIndexAction = new QAction(tr("Create a index"), this);
    createNewIndexAction->setIcon(QIcon(":/images/createindex.png"));
    createNewIndexAction->setStatusTip(tr("Create a index on Table"));
    connect(createNewIndexAction, SIGNAL(triggered()), this, SLOT(createIndex()));

//    displayIndexAction = new QAction(tr("Dispaly the index"), this);
//    displayIndexAction->setIcon(QIcon(":/images/displayindex.png"));
//    displayIndexAction->setStatusTip(tr("Display the index on SQL"));
//    connect(displayIndexAction, SIGNAL(triggered()), this, SLOT(displayIndex()));

    deleteIndexAction = new QAction(tr("Delete the index"), this);
    deleteIndexAction->setIcon(QIcon(":/images/deleteindex.png"));
    deleteIndexAction->setStatusTip(tr("Delete the current select index"));
    connect(deleteIndexAction, SIGNAL(triggered()), this, SLOT(deleteIndex()));

    dropTriggerAction = new QAction(tr("Drop the Trigger"), this);
    dropTriggerAction->setIcon(QIcon(":/images/deleteindex.png"));
    dropTriggerAction->setStatusTip(tr("Drop the current select Trigger"));
    connect(dropTriggerAction, SIGNAL(triggered()), this, SLOT(dropTrigger()));

    dropViewAction = new QAction(tr("Drop the View"), this);
    dropViewAction->setIcon(QIcon(":/images/deleteindex.png"));
    dropViewAction->setStatusTip(tr("Drop the current select View"));
    connect(dropViewAction, SIGNAL(triggered()), this, SLOT(dropView()));

    exportAction = new QAction(tr("Export"), this);
    exportAction->setStatusTip(tr("Export the Table data"));
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportData()));

    importAction = new QAction(tr("Import"), this);
    importAction->setStatusTip(tr("Import the data to Database"));
    connect(importAction, SIGNAL(triggered()), this, SLOT(importData()));

    analyzeAction = new QAction(tr("Analyze"), this);
    analyzeAction->setStatusTip(tr("Analyze the database"));
    connect(analyzeAction, SIGNAL(triggered()), this, SLOT(analyzeDatabase()));

    vacuumAction = new QAction(tr("Vacuum"), this);
    vacuumAction->setStatusTip(tr("Vacuum the database file"));
    connect(vacuumAction, SIGNAL(triggered()), this, SLOT(vacuumDatabase()));


    languageGroupAction = new QActionGroup(this);
    QAction *chineseAction = languageGroupAction->addAction(tr("Chinese"));
    chineseAction->setCheckable(true);
    QAction *englishAction = languageGroupAction->addAction(tr("English"));
    englishAction->setCheckable(true);

    QSettings settings("SQLite Database Manage.ini", QSettings::IniFormat);
    int b = settings.value("Language").toInt();


    if (b == 0)
        chineseAction->setChecked(true);
    else if (b == 1)
        englishAction->setChecked(true);
    else
        chineseAction->setChecked(true);

    connect(languageGroupAction, SIGNAL(triggered(QAction*)), this, SLOT(languageChanged(QAction*)));

    styleGroupAction = new QActionGroup(this);

//    qApp->setStyle("Plastique");
    styleGroupAction->addAction("Plastique")->setCheckable(true);
    styleGroupAction->addAction("Cleanlooks")->setCheckable(true);
    styleGroupAction->addAction("Windows")->setCheckable(true);
    styleGroupAction->addAction("WindowsXp")->setCheckable(true);
    QAction *windowsVistaAction = styleGroupAction->addAction("WindowsVista");
    windowsVistaAction->setCheckable(true);
    windowsVistaAction->setChecked(true);
    styleGroupAction->addAction("Motif")->setCheckable(true);
    styleGroupAction->addAction("CDE")->setCheckable(true);
    connect(styleGroupAction, SIGNAL(triggered(QAction*)), this, SLOT(styleChanged(QAction*)));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);    
    recentFileMenu = fileMenu->addMenu(tr("Recent Database"));
    for (int i = 0; i < MaxRecentFiles; ++i)
        recentFileMenu->addAction(recentFileAction[i]);

    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAllAction);
    fileMenu->addSeparator();
    fileMenu->addAction(createNewSQLAction);
    fileMenu->addAction(saveSqlAction);
    fileMenu->addAction(openSqlAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    editMenu = menuBar()->addMenu(tr("&Edit"));
//    editMenu->addAction(createNewTableAction);
//    editMenu->addAction(renameTableAction);
//    editMenu->addAction(deleteTableAction);
//    editMenu->addAction(modifyTableFieldAction);

//    editMenu->addSeparator();

    editMenu->addAction(cutAction);
    editMenu->addAction(copyAction);
    editMenu->addAction(pasteAction);
    editMenu->addAction(deleteAction);

    editMenu->addSeparator();

    editMenu->addAction(insertRowAction);
    editMenu->addAction(deleteRowAction);

    editMenu->addSeparator();

    editMenu->addAction(findAction);

    databaseMenu = menuBar()->addMenu(tr("&Database"));
    databaseMenu->addAction(createNewTableAction);
    databaseMenu->addAction(renameTableAction);
    databaseMenu->addAction(deleteTableAction);
    databaseMenu->addAction(modifyTableFieldAction);

    databaseMenu->addSeparator();

    databaseMenu->addAction(createNewIndexAction);
    databaseMenu->addAction(deleteIndexAction);

    executeMenu = menuBar()->addMenu(tr("&Execute"));
    executeMenu->addAction(analysisSqlAction);
    executeMenu->addAction(runLineSqlAction);
    executeMenu->addAction(runSqlAction);
//    executeMenu->addAction(createNewQueryAction);
//    executeMenu->addAction(saveSqlAction);
//    executeMenu->addAction(openSqlAction);

//    executeMenu->addSeparator();

//    executeMenu->addAction(createNewIndexAction);
//    executeMenu->addAction(displayIndexAction);
//    executeMenu->addAction(deleteIndexAction);

    optionsMenu = menuBar()->addMenu(tr("&Options"));
    optionsMenu->addAction(showGridAction);
    optionsMenu->addAction(readOnlyAction);
    optionsMenu->addAction(showDockWidgetAction);
    optionsMenu->addAction(associateAction);

    dataMenu = menuBar()->addMenu(tr("Tools"));
    dataMenu->addAction(exportAction);
    dataMenu->addAction(importAction);

    dataMenu->addSeparator();

    dataMenu->addAction(analyzeAction);
    dataMenu->addAction(vacuumAction);

    helpMenu = menuBar()->addMenu(tr("Help"));
    helpMenu->addMenu(tr("Language"))->addActions(languageGroupAction->actions());
    helpMenu->addMenu(tr("&Style"))->addActions(styleGroupAction->actions());
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::showTreeContextMenu()
{
    QMenu menu;
    menu.addAction(createNewSQLAction);
    menu.addAction(closeConnectAction);
    menu.exec(QCursor::pos());
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("&File"));
    fileToolBar->setObjectName("fileToolBar");
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(openAction);
    fileToolBar->addAction(saveAction);
    fileToolBar->addAction(saveAllAction);

    editToolBar = addToolBar(tr("&Edit"));
    editToolBar->setObjectName("editToolBar");
    editToolBar->addAction(cutAction);
    editToolBar->addAction(copyAction);
    editToolBar->addAction(pasteAction);
    editToolBar->addAction(deleteAction);
    editToolBar->addAction(findAction);
    editToolBar->addAction(insertRowAction);
    editToolBar->addAction(deleteRowAction);
    editToolBar->addSeparator();
    editToolBar->addAction(createNewTableAction);
    editToolBar->addAction(renameTableAction);
    editToolBar->addAction(deleteTableAction);
    editToolBar->addAction(modifyTableFieldAction);

    executeToolBar = addToolBar(tr("&Execute"));
    executeToolBar->setObjectName("excuteToolBar");
//    QHBoxLayout *hboxLayout = new QHBoxLayout;
//    hboxLayout->addWidget();
//    hboxLayout->addWidget();

    comboBox = new QComboBox;
    comboBox->setMinimumWidth(120);
    connect(comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(queryChanged(QString)));
    executeToolBar->addWidget(new QLabel(tr("Database:")));
    executeToolBar->addWidget(comboBox);
    executeToolBar->addSeparator();
    executeToolBar->addAction(analysisSqlAction);
    executeToolBar->addAction(runLineSqlAction);
    executeToolBar->addAction(runSqlAction);
    executeToolBar->addAction(createNewSQLAction);
    executeToolBar->addAction(saveSqlAction);
    executeToolBar->addAction(openSqlAction);
    executeToolBar->addSeparator();
    executeToolBar->addAction(createNewIndexAction);
//    executeToolBar->addAction(displayIndexAction);
    executeToolBar->addAction(deleteIndexAction);
}

void MainWindow::createStatusBar()
{
    statusBar();
}

void MainWindow::openDatabaseFile()
{
    QStringList databaseName = QFileDialog::getOpenFileNames(this,
                               tr("Open Database file"), ".",
                               tr("Database files (*.db)"));
    if (!databaseName.isEmpty())
    {
        setTreeData(databaseName);
    }

}

bool MainWindow::createConnect(const QString &databaseName)
{
    if (!QFile::exists(databaseName))
        return false;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", databaseName);
        db.setDatabaseName(databaseName);
        if (db.open())
        {
            QSettings settings("SQLite Database Manage.ini", QSettings::IniFormat);
            settings.beginGroup("mainWindow");
            QStringList files = settings.value("RecentFileList").toStringList();
            files.removeAll(databaseName);
            files.prepend(databaseName);
            while (files.size() > MaxRecentFiles)
                files.removeLast();

            //        qDebug() << files;
            settings.setValue("RecentFileList", files);

            updateRecentFileActions();
            settings.endGroup();
            return true;
        }
    }

    QSqlDatabase::removeDatabase(databaseName);
    return false;
}

bool MainWindow::findExist(const QStringList &databaseName)
{
    for (int i = 0; i < databaseName.size(); ++i)
    {
        if (QSqlDatabase::database(databaseName[i]).connectionName() == databaseName[i])
        {
            return true;
        }
    }
    return false;
}

void MainWindow::setTreeData(const QStringList &databaseName)
{
    if (findExist(databaseName))
    {
        return;
    }

    for (int i = 0; i < databaseName.size(); ++i)
    {
        if (createConnect(databaseName[i]))
        {
            int index = databaseName[i].lastIndexOf("\\");
            comboBox->addItem(databaseName[i].mid(index + 1), Qt::DisplayRole);

            QTreeWidgetItem *rootItem = new QTreeWidgetItem(treeWidget, QStringList(databaseName[i]));
            rootItem->setIcon(0, QIcon(":/images/database.png"));

            //table
            QTreeWidgetItem *tablesItem = new QTreeWidgetItem(QStringList() << tr("Tables"));
            tablesItem->setIcon(0, QIcon(":/images/tablemultiple.png"));

            QTreeWidgetItem *systemTableItem = new QTreeWidgetItem(QStringList() << tr("System Table"));
            systemTableItem->setIcon(0, QIcon(":/images/tablemultiple.png"));
//            QTreeWidgetItem *item1 = new QTreeWidgetItem(QStringList() << tr("sqlite_master"));
//            systemTableItem->addChild(item1);

            QTreeWidgetItem *viewsItem = new QTreeWidgetItem(QStringList() << tr("Views"));
            viewsItem->setIcon(0, QIcon(":/images/view.png"));

            rootItem->addChild(tablesItem);
            rootItem->addChild(systemTableItem);
            rootItem->addChild(viewsItem);

            createTableTreeItem(databaseName[i], tablesItem, systemTableItem);
            createViewTreeItem(databaseName[i], viewsItem);
//            QSqlDatabase db = QSqlDatabase::database(databaseName[i]);
//            QStringList tablesName = db.tables();
//            for (int j = 0; j < tablesName.size(); ++j)
//            {
//                QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(tablesName[j]));
//                tablesItem->addChild(item);

//                // field
//                QTreeWidgetItem *fieldItem = new QTreeWidgetItem(QStringList() << tr("Columns"));
//                fieldItem->setIcon(0, QIcon(":/images/columns.png"));
//                item->addChild(fieldItem);

//                // index
//                QTreeWidgetItem *indexItem = new QTreeWidgetItem(QStringList() << tr("Indexs"));
//                indexItem->setIcon(0, QIcon(":/images/index.png"));
//                item->addChild(indexItem);

//                QSqlQuery query(db);
//                query.exec(tr("select name from sqlite_master where type='index' and tbl_name='%1'").arg(tablesName[j]));
//                while (query.next())
//                {
//                    QTreeWidgetItem *item = new QTreeWidgetItem(query.value(0).toStringList());
//                    indexItem->addChild(item);
//                }

//                // Triggers
//                QTreeWidgetItem *triggersItem = new QTreeWidgetItem(QStringList() << tr("Triggers"));
//                triggersItem->setIcon(0, QIcon(":/images/trigger.png"));
//                item->addChild(triggersItem);

//                QSqlRecord record = db.record(tablesName[j]);

//                for (int k = 0; k < record.count(); ++k)
//                {
//                    fieldItem->addChild(new QTreeWidgetItem(QStringList(record.fieldName(k))));
//                }
//            }
        }
    }
}

void MainWindow::treeItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item && item->parent() && item->parent()->parent())
    {
        QString str = item->parent()->text(column);
        QString tableName = item->text(column);
        QString connectName = item->parent()->parent()->text(column);
        if (str == tr("Tables"))
        {
            tabWidget->createNewTableView(connectName, tableName);
        }
        else if (str == tr("System Table") || str == tr("Views"))
        {
            tabWidget->createSystemTable(connectName, tableName);
        }
    }
}

void MainWindow::setReadOnly(bool isRead)
{
    isReadOnlyMenu = isRead;
    tabWidget->setReadOnly(isRead);
}

void MainWindow::newDatabase()
{
    NewDialog dialog(this);
    if (dialog.exec())
    {
        if (dialog.checkState())
        {
            setTreeData(QStringList(dialog.fileLocationName() + dialog.fileName()));
        }
        else
        {
            QString databaseName = dialog.fileLocationName() + dialog.fileName();
            createConnect(databaseName);
            QSqlDatabase::removeDatabase(databaseName);
        }
    }
}

void MainWindow::openSql()
{
    QTreeWidgetItem *item = treeWidget->topLevelItem(0);
    if (item)
    {
        tabWidget->openSql(item->text(0));
    }
}

void MainWindow::createNewTable()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (item && item->text(0) == tr("Tables"))
    {
        QString connectName = item->parent()->text(0);
        tabWidget->createNewTable(connectName);
    }
}

void MainWindow::setTableAction(bool b)
{
//    createNewTableAction->setEnabled(b);
    renameTableAction->setEnabled(b);
    deleteTableAction->setEnabled(b);
    modifyTableFieldAction->setEnabled(b);
}

void MainWindow::setTableIndexAction(bool c, bool d)
{
    createNewIndexAction->setEnabled(c);
    deleteIndexAction->setEnabled(d);
}

void MainWindow::setAction(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parentItem = item->parent();
    if (!parentItem)
    {
        createNewTableAction->setEnabled(false);
        setTableAction(false);
        setTableIndexAction(false, false);
    }
    else if (item->text(0) == tr("Tables"))
    {
        createNewTableAction->setEnabled(true);
        setTableAction(false);
        setTableIndexAction(false, false);
    }
    else if (item->text(0) == tr("Indexs"))
    {
        createNewTableAction->setEnabled(false);
        setTableAction(false);
        setTableIndexAction(true, false);
    }
    else if (item->parent()->text(0) == tr("Tables"))
    {
        createNewTableAction->setEnabled(false);
        setTableAction(true);
        setTableIndexAction(true, false);
    }
    else if (item->parent()->text(0) == tr("Indexs"))
    {
        createNewTableAction->setEnabled(false);
        setTableAction(false);
        setTableIndexAction(false, true);
    }
    else
    {
        createNewTableAction->setEnabled(false);
        setTableAction(false);
        setTableIndexAction(false, false);
    }
    createNewSQLAction->setEnabled(true);
    return;
}

void MainWindow::slotShowTreeContextMenu(QTreeWidgetItem *item, int column)
{
    setAction(item);
    if (qApp->mouseButtons() == Qt::LeftButton)
    {
        return;
    }
    else if (qApp->mouseButtons() == Qt::RightButton && item->parent() == 0)
    {
        showTreeContextMenu();
    }
    else if (qApp->mouseButtons() == Qt::RightButton && item->text(column) == tr("Tables"))
    {
        QMenu menu;
        menu.addAction(createNewTableAction);
        menu.addAction(refreshAction);
        menu.exec(QCursor::pos());
    }
    else if (qApp->mouseButtons() == Qt::RightButton && item->text(column) == tr("Views"))
    {
        QMenu menu;
        menu.addAction(refreshAction);
        menu.exec(QCursor::pos());
    }
    else if (qApp->mouseButtons() == Qt::RightButton && item->parent()
            && item->parent()->parent())
    {
        QString str = item->parent()->text(column);
        QMenu menu;
        if (str == tr("Tables"))
        {
            menu.addAction(openTableAction);
            menu.addAction(renameTableAction);
            menu.addAction(deleteTableAction);
            menu.addAction(modifyTableFieldAction);
            menu.addSeparator();
            menu.addAction(createNewIndexAction);
        }
        else if (str == tr("Indexs"))
        {
//            menu.addAction(displayIndexAction);
            menu.addAction(deleteIndexAction);
        }
        else if (str == tr("Triggers"))
        {
            menu.addAction(dropTriggerAction);
        }
        else if (item->text(0) == tr("Indexs"))
        {
            menu.addAction(createNewIndexAction);
        }
        else if (str == tr("Views"))
        {
            menu.addAction(dropViewAction);
        }
        menu.exec(QCursor::pos());
    }
}

void MainWindow::createIndex()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (item)
    {
        if (item->text(0) == tr("Indexs"))
        {
            item = item->parent();
        }
        if (item && item->parent() && item->parent()->text(0) == tr("Tables"))
        {
            QString connectName = item->parent()->parent()->text(0);
            QString tableName = item->text(0);
            tabWidget->createIndex(connectName, tableName);
        }
    }
}

void MainWindow::displayIndex()
{
//    if (currentTreeItem)
//    {
//        QTreeWidgetItem *currentItem = currentTreeItem->treeWidget()->currentItem();

//        if (currentItem && currentItem->parent()
//                && currentItem->parent()->text(0) == tr("Indexs")
//                && currentItem->parent()->parent() == currentTreeItem)
//        {
//            QSqlDatabase db = QSqlDatabase::database(currentTreeItem->text(0));
//            QSqlQuery query(db);
//            query.exec(tr("select sql from sqlite_master where type='index' and name='%1'")
//                       .arg(currentItem->text(0)));
//            if (!query.isActive())
//            {
//                QMessageBox::information(this, tr("SQLite GUI"), query.lastError().text());
//                return;
//            }
//            QString str;
//            while (query.next())
//            {
//                str.append(query.value(0).toString());
//            }
////            sqlCodeEditor->setPlainText(str);
//            sqlTextShowAction->setChecked(true);
////            groupBox->setShown(true);
//        }
//    }
}

void MainWindow::deleteIndex()
{
    QTreeWidgetItem *item = treeWidget->currentItem();

    if (item && item->parent() && item->parent()->text(0) == tr("Indexs"))
    {
        QString indexName = item->text(0);
        int r = QMessageBox::warning(this, tr("SQLite Database Manage"),
                                     tr("Are you sure delete index %1?").arg(indexName),
                                     QMessageBox::Yes | QMessageBox::Default,
                                     QMessageBox::No,
                                     QMessageBox::Cancel | QMessageBox::Escape);

        if (r == QMessageBox::No || r == QMessageBox::Cancel)
            return;

        QString connectName = item->parent()->parent()->parent()->parent()->text(0);
        QSqlDatabase db = QSqlDatabase::database(connectName);
        QSqlQuery query(db);
        query.exec("drop index '" + indexName + "'");
        if (!query.isActive())
        {
            QMessageBox::information(this, tr("SQLite Database Manage"), query.lastError().text());
            return;
        }
        treeWidget->removeItemWidget(item, 0);
        QTreeWidgetItem *itemParent = item->parent();
        itemParent->removeChild(item);
        delete item;
    }
}

void MainWindow::deleteTable()
{

    QTreeWidgetItem *currentItem = treeWidget->currentItem();
    if (!currentItem || !(currentItem->parent()))
        return;

    if (currentItem->parent()->text(0) == tr("Tables"))
    {
        QString connectName = currentItem->parent()->parent()->text(0);
        QString tableName = currentItem->text(0);

        int r = QMessageBox::warning(this, tr("SQLite Database Manage"),
                                     tr("Are you sure delete table %1?").arg(tableName),
                                     QMessageBox::Yes | QMessageBox::Default,
                                     QMessageBox::No,
                                     QMessageBox::Cancel | QMessageBox::Escape);

        if (r == QMessageBox::No || r == QMessageBox::Cancel)
            return;


        QSqlDatabase db = QSqlDatabase::database(connectName);
        QSqlQuery query(db);
        query.exec("drop table '" + tableName + "'");
        if (!query.isActive())
        {
            QMessageBox::information(this, tr("SQLite Database Manage"), query.lastError().text());
            return;
        }
        treeWidget->removeItemWidget(currentItem, 0);
        QTreeWidgetItem *currentItemParent = currentItem->parent();
        currentItemParent->removeChild(currentItem);
        delete currentItem;
    }
}

void MainWindow::setShowGrid(bool isShow)
{
    isShowMenu = isShow;
    tabWidget->setShowGrid(isShow);
}

void MainWindow::readSettings()
{
//    QSettings settings("Finalcheat", "SQLite");
    QSettings settings("SQLite Database Manage.ini", QSettings::IniFormat);

    settings.beginGroup("mainWindow");
    QString style = settings.value("Style").toString();
    if (!style.isEmpty())
    {
        QList<QAction*> actionList = styleGroupAction->actions();
        for (int i = 0; i < actionList.size(); ++i)
        {
            if (actionList[i]->text() == style)
            {
                actionList[i]->setChecked(true);
                break;
            }
        }
        qApp->setStyle(style);
    }
    restoreGeometry(settings.value("geometry").toByteArray());
    QVariant value = settings.value("state");
    if (value.isNull())
        resize(1266, 677);
    else
        restoreState(value.toByteArray());

    rightSplitter->restoreState(settings.value("rightSplitter").toByteArray());
    dockWidget->restoreGeometry(settings.value("dockWidget").toByteArray());

    value = settings.value("showGrid");
    if (!value.isNull())
    {
        showGridAction->setChecked(value.toBool());
        isShowMenu = value.toBool();
    }

    value = settings.value("readOnly");
    if (!value.isNull())
    {
        readOnlyAction->setChecked(value.toBool());
        isReadOnlyMenu = value.toBool();
    }

    value = settings.value("showDockWidget");
    if (!value.isNull())
    {
        showDockWidgetAction->setChecked(value.toBool());
        dockWidget->setShown(value.toBool());
    }

    QStringList fileList = settings.value("CurrentFileList").toStringList();
    setTreeData(fileList);

//    fileList = settings.value("RecentFileList").toStringList();
//    while (fileList.size() > MaxRecentFiles)
//        fileList.removeLast();
//    currentFile = fileList.size();
//    for (int i = 0; i < fileList.size(); ++i)
//    {
//        QString text = tr("&%1 %2").arg(i + 1).arg(fileList[i]);
//        recentFileAction[i]->setText(text);
//        recentFileAction[i]->setData(fileList[i]);
//        recentFileAction[i]->setVisible(true);
//    }
//    recentFileMenu->setVisible(currentFile > 0);
    updateRecentFileActions();

    settings.endGroup();
}

void MainWindow::writeSettings()
{
//    QSettings settings("Finalcheat", "SQLite");
    QSettings settings("SQLite Database Manage.ini", QSettings::IniFormat);

    settings.beginGroup("mainWindow");
    settings.setValue("Style", styleGroupAction->checkedAction()->text());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("dockWidget", dockWidget->saveGeometry());
    settings.setValue("rightSplitter", rightSplitter->saveState());
    settings.setValue("showGrid", showGridAction->isChecked());
    settings.setValue("readOnly", readOnlyAction->isChecked());
    settings.setValue("showDockWidget", showDockWidgetAction->isChecked());

    QStringList fileList;
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        fileList.push_back(treeWidget->topLevelItem(i)->data(0, Qt::DisplayRole).toString());
    }
    settings.setValue("CurrentFileList", fileList);

    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
//    int r = QMessageBox::warning(this, tr("SQLite GUI"),
//                                 tr("Do you want to save your changes?\n"),
//                                 QMessageBox::Yes | QMessageBox::Default,
//                                 QMessageBox::No,
//                                 QMessageBox::Cancel | QMessageBox::Escape);

//    if (r == QMessageBox::Cancel)
//    {
//        event->ignore();
//        return;
//    }
//    else if (r == QMessageBox::Yes)
//    {
////        tabWidget->saveAll();
//    }

    writeSettings();
    event->accept();
}

void MainWindow::find()
{
    if (!findDialog)
    {
        findDialog = new FindDialog(this);
        connect(findDialog, SIGNAL(findNext(const QString &,
                                            Qt::CaseSensitivity)),
                tabWidget, SLOT(findNext(const QString &,
                                         Qt::CaseSensitivity)));
        connect(findDialog, SIGNAL(findPrevious(const QString &,
                                                Qt::CaseSensitivity)),
                tabWidget, SLOT(findPrevious(const QString &,
                                             Qt::CaseSensitivity)));

    }

    findDialog->show();
    findDialog->activateWindow();
}

void MainWindow::closeDatabaseConnect()
{
    QTreeWidgetItem *item = treeWidget->currentItem();

    QString connectName = item->text(0);
    int index = connectName.lastIndexOf("\\");
    index = comboBox->findText(connectName.mid(index + 1));
    comboBox->removeItem(index);
    QSqlDatabase::removeDatabase(connectName);
    delete item;
}

void MainWindow::renameTheTable()
{
    QTreeWidgetItem *currentItem = treeWidget->currentItem();

    if (!currentItem || !(currentItem->parent()))
        return;

    if (currentItem->parent()->text(0) == tr("Tables"))
    {
        QString tableName = currentItem->text(0);
        QString newTableName = QInputDialog::getText(treeWidget, tr("Edit Table Name"),
                                                     tr("Enter new table name:"),
                                                     QLineEdit::Normal, tableName);

        if (!newTableName.isEmpty() && tableName != newTableName)
        {
            QSqlDatabase db = QSqlDatabase::database(currentItem->parent()->parent()->text(0));
            QSqlQuery query(db);
            //ALTER TABLE oldTableName RENAME TO newTableName
            QString str = tr("alter table %1 rename to %2").arg(tableName).arg(newTableName);
            query.exec(str);
            if (!query.isActive())
            {
                QMessageBox::information(treeWidget, tr("SQLite Database Manage"),
                                         query.lastError().text());
                return;
            }
            currentItem->setText(0, newTableName);
            //        tabWidget->renameTable(tableName, newTableName);
        }
    }
}

void MainWindow::openNewTable()
{
    treeItemDoubleClicked(treeWidget->currentItem(), 0);
}

void MainWindow::modifyTableField()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (item && item->parent() && item->parent()->text(0) == tr("Tables"))
    {
        QString tableName = item->text(0);
        QString connectName = item->parent()->parent()->text(0);
        tabWidget->createModifyTableField(connectName, tableName);
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About SQLite Database Manage by Finalcheat"),
            tr("<h2>SQLite Database Manage Beta0.1</h2>"
               "<p>Copyright &copy; 2012 Software Inc."
               "<p>SQLite Database Manage is a small application."
               ));
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
    {
        QList<QUrl> urls = event->mimeData()->urls();
        for (int i = 0; i < urls.size(); ++i)
        {
            QString fileName = urls[i].toLocalFile();
            if (fileName.endsWith(".db"))
            {
                event->acceptProposedAction();
                break;
            }
        }
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    QStringList fileNameList;
    for (int i = 0; i < urls.size(); ++i)
    {
        QString fileName = urls[i].toLocalFile();        
        if (fileName.endsWith(".db"))
        {
            fileName.replace('/', "\\");
            fileNameList.push_back(fileName);
        }
    }
    setTreeData(fileNameList);
}

void MainWindow::showDockWidget(bool b)
{
    if (b)
        dockWidget->show();
    else
        dockWidget->hide();
}

void MainWindow::styleChanged(QAction *action)
{
    QApplication::setStyle(action->text());
}

void MainWindow::exportData()
{
    QMap<QString, QString> str;
    for (int i = 0; i < comboBox->count(); ++i)
    {
        str.insert(comboBox->itemText(i), treeWidget->topLevelItem(i)->text(0));
    }

    ExportWizard dialog(str);
    dialog.exec();
}

void MainWindow::createNewQuery()
{
    QTreeWidgetItem *currentItem = treeWidget->currentItem();
    if (currentItem)
    {
        while (currentItem->parent())
            currentItem = currentItem->parent();

        QString connectName = currentItem->text(0);
        int index = treeWidget->indexOfTopLevelItem(currentItem);
        comboBox->setCurrentIndex(index);
        tabWidget->createNewQuery(connectName);
    }
}

void MainWindow::refreshData()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (item)
    {
        QList<QTreeWidgetItem*> itemList = item->takeChildren();
        for (int i = 0; i < itemList.size(); ++i)
            delete itemList[i];

        QString str = item->text(0);
        if (str == tr("Tables"))
        {
            QTreeWidgetItem *systemTableItem =  item->parent()->child(1);
            QList<QTreeWidgetItem*> itemList = systemTableItem->takeChildren();
            for (int i = 0; i < itemList.size(); ++i)
                delete itemList[i];

            createTableTreeItem(item->parent()->text(0), item, systemTableItem);
        }
        else
        {
            createViewTreeItem(item->parent()->text(0), item);
        }
    }
}

void MainWindow::createTableTreeItem(const QString &connectName, QTreeWidgetItem *parentItem, QTreeWidgetItem *systemTableItem)
{   
    QSqlDatabase db = QSqlDatabase::database(connectName);
    QStringList tablesName = db.tables(QSql::SystemTables);
    tablesName += db.tables(QSql::Tables);
//    qDebug() << db.tables();
    db.transaction();
    for (int j = 0; j < tablesName.size(); ++j)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(tablesName[j]));
        if (tablesName[j].startsWith("sqlite_"))
        {
            systemTableItem->addChild(item);
        }
        else
        {
            parentItem->addChild(item);
        }

        // field
        QTreeWidgetItem *fieldItem = new QTreeWidgetItem(QStringList() << tr("Columns"));
        fieldItem->setIcon(0, QIcon(":/images/columns.png"));
        item->addChild(fieldItem);

        // index
        QTreeWidgetItem *indexItem = new QTreeWidgetItem(QStringList() << tr("Indexs"));
        indexItem->setIcon(0, QIcon(":/images/index.png"));
        item->addChild(indexItem);

        // System Index
        QTreeWidgetItem *systemIndexItem = new QTreeWidgetItem(QStringList() << tr("System Indexs"));
        systemIndexItem->setIcon(0, QIcon(":/images/index.png"));
        item->addChild(systemIndexItem);

        QSqlQuery query(db);
        query.exec(tr("select name,sql from sqlite_master where type='index' and tbl_name='%1'").arg(tablesName[j]));
        while (query.next())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(query.value(0).toStringList());
            bool isSystemIndex = query.value(1).isNull();
            if (isSystemIndex)
            {
                systemIndexItem->addChild(item);
            }
            else
            {
                indexItem->addChild(item);
            }
        }

        // Triggers
        QTreeWidgetItem *triggersItem = new QTreeWidgetItem(QStringList() << tr("Triggers"));
        triggersItem->setIcon(0, QIcon(":/images/trigger.png"));
        item->addChild(triggersItem);

        query.exec(tr("select name from sqlite_master where type='trigger' and tbl_name='%1'").arg(tablesName[j]));
        while (query.next())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(query.value(0).toStringList());
            triggersItem->addChild(item);
        }

        QSqlRecord record = db.record(tablesName[j]);

        QSqlIndex primaryName = db.primaryIndex(tablesName[j]);
        for (int k = 0; k < record.count(); ++k)
        {
            QString fieldName = record.fieldName(k);
            QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(fieldName));
            fieldItem->addChild(item);
            if (primaryName.contains(fieldName))
                item->setIcon(0, QIcon(":/images/key.png"));
        }
    }
    db.commit();
}

void MainWindow::createViewTreeItem(const QString &connectName, QTreeWidgetItem *parentItem)
{
    QSqlDatabase db = QSqlDatabase::database(connectName);

//    QSqlQuery query(db);
//    query.exec("select tbl_name from sqlite_master where type='view'");

//    while (query.next())
//    {
//        QString viewName = query.value(0).toString();
////        qDebug() << viewName;
//        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(viewName));
//        parentItem->addChild(item);
//    }
    QStringList viewsName = db.tables(QSql::Views);
//    qDebug() << viewsName;
    if (!viewsName.isEmpty())
    {
        for (int i = 0; i < viewsName.size(); ++i)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(viewsName[i]));
            parentItem->addChild(item);
        }
    }

}

void MainWindow::dropTrigger()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    QString triggerName = item->text(0);

    int r = QMessageBox::warning(this, tr("SQLite Database Manage"),
                                 tr("Are you sure delete trigger %1?").arg(triggerName),
                                 QMessageBox::Yes | QMessageBox::Default,
                                 QMessageBox::No,
                                 QMessageBox::Cancel | QMessageBox::Escape);

    if (r == QMessageBox::No || r == QMessageBox::Cancel)
        return;

    QString connectName = item->parent()->parent()->parent()->parent()->text(0);
    QSqlDatabase db = QSqlDatabase::database(connectName);
    QSqlQuery query(db);
    query.exec(tr("drop trigger '%1'").arg(triggerName));
    QTreeWidgetItem *parent = item->parent();
    parent->removeChild(item);
    delete item;
}

void MainWindow::dropView()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    QString viewName = item->text(0);

    int r = QMessageBox::warning(this, tr("SQLite Database Manage"),
                                 tr("Are you sure delete view %1?").arg(viewName),
                                 QMessageBox::Yes | QMessageBox::Default,
                                 QMessageBox::No,
                                 QMessageBox::Cancel | QMessageBox::Escape);

    if (r == QMessageBox::No || r == QMessageBox::Cancel)
        return;

    QString connectName = item->parent()->parent()->text(0);
    QSqlDatabase db = QSqlDatabase::database(connectName);
    QSqlQuery query(db);
    query.exec(tr("drop view '%1'").arg(viewName));
    QTreeWidgetItem *parent = item->parent();
    parent->removeChild(item);
    delete item;
}

void MainWindow::queryChanged(const QString &database)
{
    int index = comboBox->findText(database);
    QTreeWidgetItem *item = treeWidget->topLevelItem(index);
    if (item)
    {
        QString connectName = item->text(0);
        tabWidget->queryChanged(database, connectName);
    }
}

void MainWindow::importData()
{
    QMap<QString, QString> str;
    for (int i = 0; i < comboBox->count(); ++i)
    {
        str.insert(comboBox->itemText(i), treeWidget->topLevelItem(i)->text(0));
    }
//    qDebug() << str.keys() << endl;
//    qDebug() << str.values() << endl;
    ImportWizard temp(str);
    temp.exec();
}

void MainWindow::Associate(bool b)
{
    isAssociate = b;
}

void MainWindow::tabWidgetChanged(WidgetType type)
{
    switch (type)
    {
    case NOWIDGET :
        setNoTabWidgetAction();
        break;
    case TABLEVIEW :
        setTableViewAction();
        break;
    case SYSTEMTABLEVIEW :
        setSystemTableViewAction();
        break;
    case QUERYWIDGET :
        setQueryWidgetAction();
        break;
    case CREATEINDEXWIDGET :
        setCreateIndexWidgetAction();
        break;
    case NEWTABLEWIDGET :
        setNewTableWidgetAction();
        break;
    default :
        setModifyTableFieldWidgetAction();
    }
}

void MainWindow::setActionPrivate(bool editBool, bool executeBool)
{
    copyAction->setEnabled(editBool);
    cutAction->setEnabled(editBool);
    pasteAction->setEnabled(editBool);
    deleteAction->setEnabled(editBool);
    insertRowAction->setEnabled(editBool);
    deleteRowAction->setEnabled(editBool);
    findAction->setEnabled(editBool);

    analysisSqlAction->setEnabled(executeBool);
    runLineSqlAction->setEnabled(executeBool);
    runSqlAction->setEnabled(executeBool);
    saveSqlAction->setEnabled(executeBool);
    openSqlAction->setEnabled(executeBool);

}

void MainWindow::setNoTabWidgetAction()
{
    setActionPrivate(false, false);
    saveAction->setEnabled(false);
    saveAllAction->setEnabled(false);
}

void MainWindow::setTableViewAction()
{
    setActionPrivate(true, false);
    saveAction->setEnabled(true);
    saveAllAction->setEnabled(true);
}

void MainWindow::setSystemTableViewAction()
{
    setActionPrivate(false, false);
    copyAction->setEnabled(true);
    saveAction->setEnabled(false);
    saveAllAction->setEnabled(true);
}

void MainWindow::setQueryWidgetAction()
{
    setActionPrivate(true, true);
    saveAction->setEnabled(false);
    saveAllAction->setEnabled(true);
}

void MainWindow::setCreateIndexWidgetAction()
{
    setActionPrivate(false, false);
    saveAction->setEnabled(false);
    saveAllAction->setEnabled(true);
}

void MainWindow::setNewTableWidgetAction()
{
    setActionPrivate(false, false);
    saveAction->setEnabled(true);
    saveAllAction->setEnabled(true);
}

void MainWindow::setModifyTableFieldWidgetAction()
{
    setActionPrivate(false, false);
    saveAction->setEnabled(false);
    saveAllAction->setEnabled(true);
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        setTreeData(action->data().toStringList());
    }
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings("SQLite Database Manage.ini", QSettings::IniFormat);
    settings.beginGroup("mainWindow");
    QStringList files = settings.value("RecentFileList").toStringList();
//    qDebug() << files;

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i)
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(files[i]);
        recentFileAction[i]->setText(text);
        recentFileAction[i]->setData(files[i]);
        recentFileAction[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileAction[j]->setVisible(false);

//    recentFileMenu->setVisible(currentFile > 0);
    settings.endGroup();
}

void MainWindow::analyzeDatabase()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (item)
    {
        while (item->parent())
            item = item->parent();

        QString connectName = item->text(0);
        QSqlDatabase db = QSqlDatabase::database(connectName);
        db.exec("analyze");
    }
}

void MainWindow::vacuumDatabase()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (item)
    {
        while (item->parent())
            item = item->parent();

        QString connectName = item->text(0);
        QSqlDatabase db = QSqlDatabase::database(connectName);
        db.exec("vacuum");
    }
}

void MainWindow::languageChanged(QAction *action)
{
    QSettings settings("SQLite Database Manage.ini", QSettings::IniFormat);
    QString languageStr = action->text();
    int value = 0;
    if (languageStr == tr("English"))
        value = 1;
    settings.setValue("Language", value);
    QMessageBox::information(this, tr("SQLite Database Manage"), tr("You must restart the software change to take effect."));
}
