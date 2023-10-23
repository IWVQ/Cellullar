#include "about.h"
#include "ui_about.h"
#include "builddatetime.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    btngroup = new QButtonGroup(this);
    btngroup->addButton(ui->pushButtonCredits);
    btngroup->addButton(ui->pushButtonLicense);
    btngroup->setExclusive(true);

    connect(btngroup, SIGNAL(buttonClicked(QAbstractButton*)), this,
            SLOT(on_btngroup_buttonClicked(QAbstractButton*)));

    ui->textEdit->setVisible(false);
    ui->labelBuildOn->setText("Build on " + BUILDDATETIME);
}

About::~About()
{
    delete ui;
}

void About::on_btngroup_buttonClicked(QAbstractButton *btn)
{
    if (ui->pushButtonCredits->isChecked()){
        QFile file(":/doc/credits.html");
        file.open(QIODevice::ReadOnly);
        QString credits = file.readAll();
        ui->textEdit->setReadOnly(false);
        ui->textEdit->clear();
        ui->textEdit->setHtml(credits);
        ui->textEdit->setVisible(true);
        ui->textEdit->setReadOnly(true);
    }
    else if (ui->pushButtonLicense->isChecked()){
        QFile file(":LICENSE");
        file.open(QIODevice::ReadOnly);
        QString license = file.readAll();
        ui->textEdit->setReadOnly(false);
        ui->textEdit->clear();
        QTextCharFormat format = QTextCharFormat();
        format.setFont(ui->textEdit->font());
        ui->textEdit->setCurrentCharFormat(format);
        ui->textEdit->setPlainText(license);
        ui->textEdit->setVisible(true);
        ui->textEdit->setReadOnly(true);
    }
    else {
        ui->textEdit->setVisible(false);
    }
}

