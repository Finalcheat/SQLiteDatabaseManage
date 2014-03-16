#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtSql/QSqlDatabase>
#include "tabwidget.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;
class QTableWidget;
class QTextEdit;
class QSplitter;
class QAction;
class QMenu;
class QToolBar;
class QTreeWidgetItem;
class QSqlQuery;
class QTableWidgetItem;
class QFont;
class TabWidget;
class FindDialog;
class QGroupBox;
class QDockWidget;
class QTreeView;
class CodeEditor;
class Highlighter;
class MyDockWidget;
class QActionGroup;
class QComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    static bool getCurrentShow() { return isShowMenu; }
    static bool getCurrentReadOnly() { return isReadOnlyMenu; }
    static bool getCurrentAssociate() { return isAssociate; }

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void showTreeContextMenu();
    bool createConnect(const QString &databaseName);
    void setTreeData(const QStringList &databaseName);
    void readSettings();
    void writeSettings();
    bool findExist(const QStringList &databaseName);
    void createTableTreeItem(const QString &connectName, QTreeWidgetItem *parentItem, QTreeWidgetItem *systemTableItem);
    void createViewTreeItem(const QString &connectName, QTreeWidgetItem *parentItem);
    void setActionPrivate(bool editBool, bool executeBool);
    void setNoTabWidgetAction();
    void setTableViewAction();
    void setSystemTableViewAction();
    void setQueryWidgetAction();
    void setCreateIndexWidgetAction();
    void setNewTableWidgetAction();
    void setModifyTableFieldWidgetAction();
    void setTableAction(bool);
    void setTableIndexAction(bool, bool);
    void setAction(QTreeWidgetItem *item);
    void updateRecentFileActions();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void openDatabaseFile();
    void openSql();
    void treeItemDoubleClicked(QTreeWidgetItem * item, int column);
    void setReadOnly(bool isRead);
    void Associate(bool);
    void slotShowTreeContextMenu(QTreeWidgetItem *, int);
    void newDatabase();
    void setShowGrid(bool isShow);
    void showDockWidget(bool);
    void find();
    void closeDatabaseConnect();
    void createNewTable();
    void openNewTable();
    void deleteTable();
    void renameTheTable();
    void modifyTableField();
    void createNewQuery();
    void createIndex();
    void displayIndex();
    void deleteIndex();
    void dropTrigger();
    void dropView();
    void styleChanged(QAction *action);
    void languageChanged(QAction *action);
    void queryChanged(const QString &database);
    void tabWidgetChanged(WidgetType type);
    void refreshData();
    void exportData();
    void importData();
    void openRecentFile();
    void analyzeDatabase();
    void vacuumDatabase();
    void about();

private:
    MyDockWidget *dockWidget;
    QTreeWidget *treeWidget;

    TabWidget *tabWidget;

    QSplitter *rightSplitter;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *databaseMenu;
    QMenu *executeMenu;
    QMenu *helpMenu;
    QMenu *optionsMenu;
    QMenu *dataMenu;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *executeToolBar;

    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAllAction;
    QAction *exitAction;

    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *deleteAction;
    QAction *insertRowAction;
    QAction *deleteRowAction;
    QAction *findAction;

    QAction *analysisSqlAction;
    QAction *runLineSqlAction;
    QAction *runSqlAction;
    QAction *saveSqlAction;
    QAction *openSqlAction;

    QAction *showGridAction;
    QAction *readOnlyAction;
    QAction *showDockWidgetAction;
    QAction *associateAction;

    QAction *exportAction;
    QAction *importAction;

    QAction *analyzeAction;
    QAction *vacuumAction;

    QActionGroup *styleGroupAction;
    QActionGroup *languageGroupAction;

    QAction *aboutAction;
    QAction *aboutQtAction;

    QAction *closeConnectAction;

    QAction *createNewTableAction;
    QAction *refreshAction;

    QAction *openTableAction;
    QAction *renameTableAction;
    QAction *deleteTableAction;
    QAction *modifyTableFieldAction;

    QAction *createNewSQLAction;
    QAction *createNewIndexAction;
//    QAction *displayIndexAction;
    QAction *deleteIndexAction;

    QAction *dropTriggerAction;

    QAction *dropViewAction;

    static bool isShowMenu;
    static bool isReadOnlyMenu;
    static bool isAssociate;

    FindDialog *findDialog;

    QComboBox *comboBox;

    QMenu *recentFileMenu;
    enum { MaxRecentFiles = 5 };
    QAction *recentFileAction[MaxRecentFiles];
};

#endif // MAINWINDOW_H
