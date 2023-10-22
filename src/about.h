#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include <QtWidgets>

namespace Ui {
class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = nullptr);
    ~About();

private slots:
    void on_btngroup_buttonClicked(QAbstractButton *btn);
private:
    Ui::About *ui;

    QButtonGroup *btngroup;
    QString credits = "";
};

#endif // ABOUT_H
