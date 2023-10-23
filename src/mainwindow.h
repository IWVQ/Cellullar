#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <math.h>
#include <QtWidgets>
#include <QMainWindow>
#include "cellularwidget.h"
#include "about.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class StateDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit StateDelegate(QObject *parent = nullptr);
    ~StateDelegate();
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    CellularWidget *cellwidget = nullptr;
    QIcon addColorIcon;
    QIcon addColorIconHot;
    QIcon delColorIcon;
    QIcon delColorIconHot;
};

class ColorListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit ColorListWidget(QWidget *parent = nullptr);
    ~ColorListWidget();
    void autoAdjustSize();
    void deleteStateItem(int index);
    void insertStateItem(int index);
    void clearStates(bool withcolors);
    int statesCount();
    CellularWidget *cellauto;
protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool autosizing = false;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushTransition_clicked();

    void on_spinStep_valueChanged(int arg1);

    void on_spinInterval_valueChanged(int arg1);

    void on_spinSizeX_valueChanged(int arg1);

    void on_spinSizeY_valueChanged(int arg1);

    void on_editProjectName_textChanged(const QString &arg1);

    void on_propButton_toggled(bool checked);

    void on_zoomInBtn_pressed();

    void on_actionSave_triggered();

    void on_actionOpen_triggered();

    void on_actionMove_triggered(bool checked);

    void on_actionDraw_triggered(bool checked);

    void on_actionErase_triggered(bool checked);

    void on_actionResume_triggered();

    void on_actionPause_triggered();

    void on_actionNew_triggered();

    void on_actionClose_triggered();

    void on_colorListWidget_itemClicked(QListWidgetItem *item);

    void on_colorListWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_colorListWidget_itemSelectionChanged();
    void on_actionZoomIn_triggered();

    void on_actionZoomOut_triggered();

    void on_actionZoom100_triggered();

    void on_zoomOutBtn_pressed();

    void on_zoomSlider_valueChanged(int value);

    void on_zoomCombo_currentTextChanged(const QString &arg1);

    void on_cellauto_scaled(double z);

    void on_actionExit_triggered();

    void on_actionClear_triggered();

    void on_cellauto_readCAMetadata(QJsonObject *metadata);

    void on_cellauto_writeCAMetadata(QJsonObject *metadata);

    void on_actionAbout_triggered();

    void on_cellauto_planning(QPoint p);

    void on_cellauto_modified();

    void on_actionTransitionRule_triggered();

protected:
    //bool eventFilter(QObject* o, QEvent* e);

private:
    void editTransitionRule();
    void loadResolution(QSize r);
    bool projectClosed();
    void saveToSomeFile(bool saveas = false);
    void openSomeFile();
    void zoomTo(double z);
    double zoomFromSlider(int v);
    int sliderFromZoom(double z);
    QString zoomString(double z);
    bool scanZoomString(QString s, double &z);
    void moveZoomSlider(int dir);
    void projectModified(bool yes = true);

    Ui::MainWindow *ui;
    CellularWidget *cellauto;
    bool modified = false;
    double proportion = NAN;
    QString currentfile = "";

    char selectedState = 0;

    QLabel *coordStatusLbl;
    QLabel *coordStatusImg;
    QWidget *coordStatusWdg;
    QHBoxLayout *coordStatusLayout;
    QLabel *modifiedStatusLbl;

    QActionGroup *editactiongroup;
    StateDelegate *statedelegate;
    QListWidgetItem *addColorItem;
    QListWidgetItem *delColorItem;
    ColorListWidget *colorListWidget;

    About *aboutdlg;
};
#endif // MAINWINDOW_H
