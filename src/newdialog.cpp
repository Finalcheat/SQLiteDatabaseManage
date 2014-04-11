#include "newdialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QApplication>
#include <QFileDialog>
#include <QCheckBox>

NewDialog::NewDialog(QWidget *parent) :
    QDialog(parent)
{
    nameLabel = new QLabel(tr("&File Name:"));
    nameLineEdit = new QLineEdit;
    nameLabel->setBuddy(nameLineEdit);
    locationsLabel = new QLabel(tr("File Locations:"));
    fileLocationStr = QApplication::applicationDirPath() + "/";
    //fileLocationStr.replace(QString(("/")), QString(("\\")));
    fileLocationsLabel = new QLabel(fileLocationStr);
    fileLocationsButton = new QPushButton(tr("Select"));
    okButton = new QPushButton(tr("OK"));
    okButton->setEnabled(false);
    cancelButton = new QPushButton(tr("Cancel"));
    checkBox = new QCheckBox(tr("Create and connect"));
    checkBox->setChecked(true);

    QGroupBox *groupBox = new QGroupBox(this);
    QGridLayout *topLayout = new QGridLayout;
    topLayout->addWidget(nameLabel, 0, 0);
    topLayout->addWidget(nameLineEdit, 0, 1, 1, 6);
    topLayout->addWidget(fileLocationsButton, 0, 7);
    topLayout->addWidget(locationsLabel, 1, 0);
    topLayout->addWidget(fileLocationsLabel, 1, 1, 1, 6);
    groupBox->setLayout(topLayout);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(checkBox);
    bottomLayout->addStretch();
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(groupBox);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);

    setWindowTitle(tr("New Database File"));

    setFixedHeight(sizeHint().height());

    connect(fileLocationsButton, SIGNAL(clicked()), this, SLOT(selectClicked()));
    connect(nameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(isTextEmpty()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void NewDialog::selectClicked()
{
    QString databaseName = QFileDialog::getSaveFileName(this,
                               tr("New Database file"), ".",
                               tr("Database files (*.db)"));
    if (!databaseName.isEmpty())
    {
        int index = databaseName.lastIndexOf("/");
        nameLineEdit->setText(databaseName.mid(index + 1));
        fileLocationStr = databaseName.left(index + 1);
//        fileLocationStr.replace(QString(("/")), QString(("\\")));
        fileLocationsLabel->setText(fileLocationStr);
    }
}

void NewDialog::isTextEmpty()
{
    okButton->setEnabled(!nameLineEdit->text().isEmpty());
}

bool NewDialog::checkState() const
{
    return checkBox->isChecked();
}

QString NewDialog::fileName() const
{
    return nameLineEdit->text();
}

QString NewDialog::fileLocationName()
{
    return fileLocationStr.replace(QString(("/")), QString(("\\")));
}
