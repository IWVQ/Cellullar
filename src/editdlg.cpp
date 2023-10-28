#include "editdlg.h"
#include "ui_editdlg.h"
#include "jshilite.h"

EditDlg::EditDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDlg)
{
    ui->setupUi(this);

    ui->replaceWidget->setVisible(false);
    ui->searchWidget->setVisible(false);


    QFont font;
    font.setFamily("Lucida console");
    font.setFixedPitch(true);
    font.setPointSize(8);

    ui->textEdit->setFont(font);
    highlighter = new JSHighlighter(ui->textEdit->document());

    ui->textEdit->setText("");

    // menu

    edtmenu = new QMenu(ui->menuToolBtn);
    edtmenu->setObjectName(QString::fromUtf8("edtmenu"));
    edtmenufile = new QMenu(edtmenu);
    edtmenufile->setObjectName(QString::fromUtf8("edtmenufile"));
    edtmenuedit = new QMenu(edtmenu);
    edtmenuedit->setObjectName(QString::fromUtf8("edtmenuedit"));

    edtmenu->setTitle("Menu");
    edtmenufile->setTitle("File");
    edtmenuedit->setTitle("Edit");


    edtmenufile->addAction(ui->actionEdtNew);
    edtmenufile->addAction(ui->actionEdtOpen);
    edtmenufile->addAction(ui->actionEdtSaveAs);

    edtmenu->addAction(edtmenufile->menuAction());

    edtmenuedit->addAction(ui->actionEdtUndo);
    edtmenuedit->addAction(ui->actionEdtRedo);
    edtmenuedit->addSeparator();
    edtmenuedit->addAction(ui->actionEdtCopy);
    edtmenuedit->addAction(ui->actionEdtPaste);
    edtmenuedit->addAction(ui->actionEdtCut);
    edtmenuedit->addSeparator();
    edtmenuedit->addAction(ui->actionEdtSelectAll);
    edtmenuedit->addAction(ui->actionEdtSearch);
    edtmenuedit->addAction(ui->actionEdtReplace);

    edtmenu->addAction(edtmenuedit->menuAction());

    edtmenu->addAction(ui->actionEdtBuild);
    edtmenu->addSeparator();
    edtmenu->addAction(ui->actionAccept);
    edtmenu->addAction(ui->actionCancel);

    ui->menuToolBtn->setMenu(edtmenu);
}

EditDlg::~EditDlg()
{
    delete ui;
}

void EditDlg::modifyRule(bool yes)
{
    rulemodified = yes;
    if (rulemodified)
        this->setWindowTitle("Transition rule - *");
    else
        this->setWindowTitle("Transition rule");
}


void EditDlg::saveRule()
{
    savedtransitionrule = ui->textEdit->toPlainText();
    modifyRule(false);
}

void EditDlg::applyConfig(QJsonDocument *config)
{
    //# a lot of things
    // - theme, language, editor,
}

QString EditDlg::transitionRule()
{
    return ui->textEdit->toPlainText();
}

void EditDlg::setTransitionRule(QString s)
{
    ui->textEdit->setPlainText(s);
    saveRule();
}

void EditDlg::showEditDialog(const QString &s)
{
    setTransitionRule(s);
    show();
    raise();
}

void EditDlg::setRunning(bool yes)
{
    QString s = "";
    if (yes){
        saveRule(); // necessary because ok/close
        s = " - [running..]";
    }
    this->setWindowTitle("Transition rule" + s);
    ui->textEdit->setReadOnly(yes);
}

void EditDlg::on_textEdit_cursorPositionChanged()
{
    int x = ui->textEdit->textCursor().columnNumber() + 1;
    int y = ui->textEdit->textCursor().blockNumber() + 1;
    ui->posLabel->setText("line: " + QString::number(y) + ", col: " + QString::number(x));
}


void EditDlg::on_pushButton_15_toggled(bool checked)
{
    if (checked)
        ui->textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    else
        ui->textEdit->setLineWrapMode(QTextEdit::NoWrap);
}


void EditDlg::on_actionEdtUndo_triggered()
{
    ui->textEdit->undo();
}


void EditDlg::on_actionEdtRedo_triggered()
{
    ui->textEdit->redo();
}


void EditDlg::on_actionEdtCut_triggered()
{
    ui->textEdit->cut();
}


void EditDlg::on_actionEdtCopy_triggered()
{
    ui->textEdit->copy();
}


void EditDlg::on_actionEdtPaste_triggered()
{
    ui->textEdit->paste();
}


void EditDlg::on_actionEdtSelectAll_triggered()
{
    ui->textEdit->selectAll();
}


void EditDlg::on_actionAccept_triggered()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->click();
}


void EditDlg::on_actionCancel_triggered()
{
    ui->buttonBox->button(QDialogButtonBox::Cancel)->click();
}


void EditDlg::on_pushButton_14_clicked()
{
    ui->actionEdtBuild->trigger();
}


void EditDlg::on_pushButton_13_clicked()
{ // settings
    emit showConfig();
}


void EditDlg::on_pushButton_12_clicked()
{ // help
    emit showHelp();
}


void EditDlg::on_actionEdtReplace_triggered()
{

}


void EditDlg::on_actionEdtSearch_triggered()
{

}


void EditDlg::on_actionEdtBuild_triggered()
{
    QString s = transitionRule();
    emit buildTransitionRule(s);
    saveRule();
}

void EditDlg::on_textEdit_textChanged()
{
    modifyRule(true);
}


void EditDlg::on_EditDlg_accepted()
{
    on_actionEdtBuild_triggered();
}


void EditDlg::on_EditDlg_rejected()
{
    setTransitionRule(savedtransitionrule);
    on_actionEdtBuild_triggered();
}

