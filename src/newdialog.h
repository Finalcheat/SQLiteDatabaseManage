#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QDialog>
class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;

class NewDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewDialog(QWidget *parent = 0);

public:
    bool checkState() const;
    QString fileLocationName() const { return fileLocationStr; }
    QString fileName() const;

private slots:
    void selectClicked();
    void isTextEmpty();

private:
    QLabel *nameLabel;
    QLabel *fileLocationsLabel;
    QLabel *locationsLabel;
    QLineEdit *nameLineEdit;
    QPushButton *fileLocationsButton;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QCheckBox *checkBox;
    QString fileLocationStr;
};

#endif // NEWDIALOG_H
