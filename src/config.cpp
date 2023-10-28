#include "config.h"
#include "ui_config.h"

Config::Config(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Config)
{
    ui->setupUi(this);

    group = new QButtonGroup(this);
    group->addButton(ui->generalBtn);
    group->addButton(ui->editorBtn);
    group->addButton(ui->automatBtn);
    group->setExclusive(true);

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this,
            SLOT(on_group_buttonClicked(QAbstractButton*)));

}

Config::~Config()
{
    delete ui;
}

void Config::showConfig(QJsonDocument *config, Subconfig sub)
{
    show();
    raise();
    loadConfig(config->object());

    if (sub == Sub_Default){}
    else if (sub == Sub_General)
        ui->generalBtn->setChecked(true);
    else if (sub == Sub_Editor)
        ui->editorBtn->setChecked(true);
    else if (sub == Sub_Automaton)
        ui->editorBtn->setChecked(true);
}

void Config::applyConfig(QJsonDocument *config)
{
    //# apply theme and language
}

void Config::on_group_buttonClicked(QAbstractButton *btn)
{
    if (ui->generalBtn->isChecked()){
        ui->stackedWidget->setCurrentIndex(0);
    }
    else if (ui->editorBtn->isChecked()){
        ui->stackedWidget->setCurrentIndex(1);
    }
    else if (ui->automatBtn->isChecked()){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void Config::on_comboBox_Theme_currentIndexChanged(const QString &arg1){}


void Config::on_buttonBox_clicked(QAbstractButton *button)
{ // reset config
    if (ui->buttonBox->button(QDialogButtonBox::Reset) == button){
        QFile loadfile("config-default.json");
        if (!loadfile.open(QIODevice::ReadOnly)) {
            qWarning("Couldn't open automaton file.");
            return;
        }
        QByteArray data = loadfile.readAll();
        QJsonDocument jdoc(QJsonDocument::fromJson(data));

        loadConfig(jdoc.object());
    }
}


void Config::on_Config_accepted()
{
    QJsonObject json;
    json["config"] = "Cellullar";

    QJsonObject general;
    general["theme"] = ui->comboBox_Theme->currentText();
    general["language"] = ui->comboBox_Language->currentText();
    json["general"] = general;

    QJsonObject editor;
    editor["font-family"] = ui->fontComboBox->currentText();
    editor["font-size"] = ui->spinBox_fontsize->value();
    editor["tab-size"] = ui->spinBox_tabsize->value();
    editor["hilite-theme"] = ui->comboBox_editortheme->currentText();
    editor["tab-to-spaces"] = ui->checkBox_tabtospaces->isChecked();
    editor["auto-indent"] = ui->checkBox_autoindent->isChecked();
    editor["line-wrap"] = ui->checkBox_linewrap->isChecked();
    json["editor"] = editor;

    QJsonObject automaton;
    automaton["counter-digits"] = ui->spinBox_counterdigits->value();
    automaton["auto-build"] = ui->checkBox_autobuild->isChecked();
    automaton["restart-counter"] = ui->checkBox_restartcounter->isChecked();
    QString hex = QString("%1").arg(automaton_background.rgb(), 8, 16, QLatin1Char('0'));
    automaton["background"] = hex;
    json["automaton"] = automaton;

    emit saveConfig(json);
}

void Config::loadConfig(QJsonObject config)
{
    QJsonValue v = config["config"];
    if (!v.isString()) return;
    if (v.toString() != "Cellullar") return;

    v = config["general"];
    if (v.isObject()){
        // general
        QJsonObject general = v.toObject();

        v = general["theme"];
        if (v.toString() == "Default")
            ui->comboBox_Theme->setCurrentIndex(0);
        else if (v.toString() == "Dark")
            ui->comboBox_Theme->setCurrentIndex(1);

        v = general["language"];
        if (v.toString() == "English")
            ui->comboBox_Language->setCurrentIndex(0);
        else if (v.toString() == "Español")
            ui->comboBox_Language->setCurrentIndex(1);
        else if (v.toString() == "Qheswa")
            ui->comboBox_Language->setCurrentIndex(2);
    }
    v = config["editor"];
    if (v.isObject()){
        // editor
        QJsonObject editor = v.toObject();

        v = editor["font-family"];
        if (v.isString()){
            QFont f = ui->fontComboBox->currentFont();
            f.setFamily(v.toString());
            ui->fontComboBox->setCurrentFont(f);
        }

        v = editor["font-size"];
        ui->spinBox_fontsize->setValue(v.toInt());

        v = editor["tab-size"];
        ui->spinBox_tabsize->setValue(v.toInt());

        v = editor["hilite-theme"];
        if (v.toString() == "Light")
            ui->comboBox_editortheme->setCurrentIndex(0);
        else if (v.toString() == "Dark")
            ui->comboBox_editortheme->setCurrentIndex(1);

        v = editor["tab-to-spaces"];
        ui->checkBox_tabtospaces->setChecked(v.toBool());

        v = editor["auto-indent"];
        ui->checkBox_autoindent->setChecked(v.toBool());

        v = editor["line-wrap"];
        ui->checkBox_linewrap->setChecked(v.toBool());
    }
    v = config["automaton"];
    if (v.isObject()){
        // automaton
        QJsonObject automaton = v.toObject();

        v = automaton["background"];
        bool ok;
        automaton_background = v.toString().toUInt(&ok, 16);
        colorIconForButton(ui->backBtnAuto, automaton_background);

        v = automaton["counter-digits"];
        ui->spinBox_counterdigits->setValue(v.toInt());

        v = automaton["restart-counter"];
        ui->checkBox_restartcounter->setChecked(v.toBool());

        v = automaton["auto-build"];
        ui->checkBox_autobuild->setChecked(v.toBool());
    }
}

void Config::colorIconForButton(QPushButton *button, QColor color)
{
    QImage img(QSize{20, 15}, QImage::Format_RGB32);
    img.fill(color);
    button->setIcon(QIcon(QPixmap::fromImage(img)));
}


void Config::on_backBtnAuto_clicked()
{
    QColor color = QColorDialog::getColor(automaton_background, this, "Color");
    automaton_background = color;
    colorIconForButton(ui->backBtnAuto, automaton_background);
}

