#ifndef CONFIG_H
#define CONFIG_H

#include <QDialog>
#include <QtWidgets>

namespace Ui {
class Config;
}

class Config : public QDialog
{
    Q_OBJECT

public:

    enum Subconfig{
        Sub_Default = 0,
        Sub_General = 1,
        Sub_Editor = 2,
        Sub_Automaton = 3
    };

    explicit Config(QWidget *parent = nullptr);
    ~Config();

    void showConfig(QJsonDocument *config, Subconfig sub = Sub_Default);
    void applyConfig(QJsonDocument *config);

signals:
    void saveConfig(const QJsonObject &config);

private slots:
    void on_group_buttonClicked(QAbstractButton *btn);
    void on_comboBox_Theme_currentIndexChanged(const QString &arg1);

    void on_buttonBox_clicked(QAbstractButton *button);

    void on_Config_accepted();

    void on_backBtnAuto_clicked();

private:
    void loadConfig(QJsonObject config);
    void colorIconForButton(QPushButton *button, QColor color);
    Ui::Config *ui;

    QColor automaton_background;
    QButtonGroup *group;
};

#endif // CONFIG_H
