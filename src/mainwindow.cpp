#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>

/* ColorListWidget */

ColorListWidget::ColorListWidget(QWidget *parent): QListWidget(parent){}

ColorListWidget::~ColorListWidget() = default;

void ColorListWidget::resizeEvent(QResizeEvent *event)
{
    QListWidget::resizeEvent(event);
    autoAdjustSize();
}

void ColorListWidget::keyPressEvent(QKeyEvent *event)
{
    QListWidget::keyPressEvent(event);
    if ((event->key() == Qt::Key_Backspace) || (event->key() == Qt::Key_Delete)){
        int index = currentRow();
        deleteStateItem(index);
    }
}

void ColorListWidget::deleteStateItem(int index)
{
    if ((0 <= index) && (index < statesCount())){
        int statecount = statesCount();
        for (int i = index; i < statecount - 1; i++)
            cellauto->setStateColor(i, cellauto->stateColor(i + 1));
        cellauto->setStateColor(statecount - 1, cellauto->stateColor(0));
        delete takeItem(index);
        autoAdjustSize();
    }
}

void ColorListWidget::insertStateItem(int index)
{
    if (index < 0) return;
    if (index > statesCount()) return;

    QListWidgetItem *newitem;
    newitem = new QListWidgetItem();
    newitem->setSizeHint(QSize{20, 20});
    newitem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    insertItem(index, newitem);
    autoAdjustSize();
    // select new state
    setItemSelected(newitem, true);
}

void ColorListWidget::clearStates(bool withcolors)
{
    if (withcolors){
        for (int i = 0; i < statesCount(); i++)
            cellauto->setStateColor(i, cellauto->stateColor(0));
    }
    int l = count();
    while (l > 2){
        delete takeItem(0);
        l--;
    }
    autoAdjustSize();
}

int ColorListWidget::statesCount()
{
    return count() - 2;
}

void ColorListWidget::autoAdjustSize()
{
    if (autosizing) return; // avoids reentrance
    autosizing = true;
    //int delta = 4;
    int delta = 0;
    int linewidth = (rect().width() - delta) / 20;
    int linescount = (count() + linewidth - 1) / linewidth;
    int h = linescount*20 + delta;
    setMinimumSize(QSize{0, h});
    setMaximumSize(QSize{16777215, h});
    autosizing = false;
}

/* ItemDelegate */

StateDelegate::StateDelegate(QObject *parent): QStyledItemDelegate(parent)
{
    addColorIcon = QIcon(":/lucide-icons/plus_l_20.png");
    addColorIconHot = QIcon(":/lucide-icons/plus-circle_b_20.png");
    delColorIcon = QIcon(":/lucide-icons/x_l_20.png");
    delColorIconHot = QIcon(":/lucide-icons/x-circle_b_20.png");
}

StateDelegate::~StateDelegate() = default;

void StateDelegate::paint(QPainter *painter,
           const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    painter->save();
    painter->setClipping(true);
    painter->setClipRect(option.rect);


    QRectF backr = option.rect;
    backr.setHeight(backr.height() - 1);
    backr.setWidth(backr.width() - 1);
    QColor highcolor = option.palette.color(QPalette::Highlight);

    int i = index.row();
    if (i == index.model()->rowCount() - 1){
        // draw close cell
        QPixmap pixmap;
        if (option.state & QStyle::State_MouseOver)
            pixmap = delColorIconHot.pixmap(QSize{20, 20});
        else
            pixmap = delColorIcon.pixmap(QSize{20, 20});
        painter->drawPixmap(option.rect, pixmap, QRect());
    }
    else if (i == index.model()->rowCount() - 2){
        // draw add cell
        QPixmap pixmap;
        if (option.state & QStyle::State_MouseOver)
            pixmap = addColorIconHot.pixmap(QSize{20, 20});
        else
            pixmap = addColorIcon.pixmap(QSize{20, 20});
        painter->drawPixmap(option.rect, pixmap, QRect());
    }
    else if (i < index.model()->rowCount() - 1){
        // draw background
        if (option.state & QStyle::State_Selected){
            highcolor.setAlpha(192);
            QPen pen(highcolor, 1);
            highcolor.setAlpha(64);
            QBrush brush(highcolor);
            painter->setPen(pen);
            painter->setBrush(brush);
            painter->drawRect(backr);
        }
        else if (option.state & QStyle::State_MouseOver){
            highcolor.setAlpha(64);
            QPen pen(highcolor, 1);
            highcolor.setAlpha(32);
            QBrush brush(highcolor);
            painter->setPen(pen);
            painter->setBrush(brush);
            painter->drawRect(backr);
        }

        // draw cell

        QRectF cellr(option.rect.left() + 2.5, option.rect.top() + 2.5,
                 option.rect.width() - 5, option.rect.height() - 5);
        painter->setRenderHint(QPainter::Antialiasing, true);
        QPen pen(Qt::black);
        painter->setPen(pen);
        QBrush brush(cellwidget->stateColor(i));
        painter->setBrush(brush);
        painter->drawRoundedRect(cellr, 2, 2, Qt::AbsoluteSize);
    }

    painter->restore();
}

QSize StateDelegate::sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const
{
    return QSize{20, 20};
}

/* MainWindow */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // colorListWidget
    colorListWidget = new ColorListWidget(ui->groupBox_State);
    colorListWidget->setObjectName(QString::fromUtf8("colorListWidget"));
    colorListWidget->setMinimumSize(QSize(0, 20));
    colorListWidget->setMaximumSize(QSize(16777215, 20));
    colorListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    colorListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    colorListWidget->setIconSize(QSize(20, 20));
    colorListWidget->setViewMode(QListView::IconMode);
    colorListWidget->setWordWrap(true);
    ui->verticalLayout_4->addWidget(colorListWidget);
    connect(colorListWidget, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(on_colorListWidget_itemClicked(QListWidgetItem *)));
    connect(colorListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(on_colorListWidget_itemDoubleClicked(QListWidgetItem *)));
    colorListWidget->setResizeMode(QListView::Adjust);
    colorListWidget->setStyleSheet("ColorListWidget{background:transparent; border:0px;}");
    //

    cellauto = new CellularWidget(ui->centralwidget);
    cellauto->setObjectName(QString::fromUtf8("cellularwidget"));
    cellauto->setVisible(true);
    cellauto->setBackColor(QColor("#d1d1d1"));
    ui->gridLayout->addWidget(cellauto, 0, 0, 1, 1);
    connect(cellauto, SIGNAL(scaled(double)), this, SLOT(on_cellauto_scaled(double)));
    connect(cellauto, SIGNAL(readCAMetadata(QJsonObject*)), this, SLOT(on_cellauto_readCAMetadata(QJsonObject*)));
    connect(cellauto, SIGNAL(writeCAMetadata(QJsonObject*)), this, SLOT(on_cellauto_writeCAMetadata(QJsonObject*)));
    cellauto->setResolution(QSize{100, 100});

    // zoom bar
    ui->zoomCombo->lineEdit()->setAlignment(Qt::AlignRight);
    ui->statusbar->addPermanentWidget(ui->zoomWidget);

    // coord status
    statusCellCoord = new QLabel(ui->toolBar);
    statusCellCoord->setObjectName(QString::fromUtf8("statusModified"));
    statusCellCoord->setVisible(true);
    statusCellCoord->setMinimumSize(QSize{125, 0});
    statusCellCoord->setMaximumSize(QSize{125, 16777215});
    statusCellCoord->setText("1230, 4322");
    ui->statusbar->addWidget(statusCellCoord);

    //
    editactiongroup = new QActionGroup(this);
    editactiongroup->addAction(ui->actionMove);
    editactiongroup->addAction(ui->actionDraw);
    editactiongroup->addAction(ui->actionErase);
    editactiongroup->setExclusive(true);

    // delegate
    statedelegate = new StateDelegate(this);
    statedelegate->cellwidget = cellauto;
    colorListWidget->setItemDelegate(statedelegate);

    // state buttons

    addColorItem = new QListWidgetItem();
    addColorItem->setSizeHint(QSize{20, 20});
    addColorItem->setFlags(Qt::ItemIsEnabled);
    colorListWidget->addItem(addColorItem);

    delColorItem = new QListWidgetItem();
    delColorItem->setSizeHint(QSize{20, 20});
    delColorItem->setFlags(Qt::ItemIsEnabled);
    colorListWidget->addItem(delColorItem);

    colorListWidget->cellauto = cellauto;
    connect(colorListWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(on_colorListWidget_itemSelectionChanged()));
    //

    ui->dockWidget->setGeometry(0, 0, 100, 100);

    // about

    aboutdlg = new About(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// own slots

// other slots

void MainWindow::on_pushTransition_clicked()
{ // configure
    cellauto->setResolution(QSize{300, 300});
    cellauto->setMode(CA_DRAW);
    cellauto->setPenState(1);
}

void MainWindow::on_spinStep_valueChanged(int arg1)
{ // step
    cellauto->setStep(arg1);
}

void MainWindow::on_spinInterval_valueChanged(int arg1)
{ // interval
    cellauto->setInterval(arg1);
}

void MainWindow::on_spinSizeX_valueChanged(int arg1)
{ // cols count (resolution)
    QSize r = cellauto->resolution();
    r.setWidth(arg1);
    if (!isnan(proportion))
        r.setHeight(int(arg1/proportion)); // W/H = w/h
    cellauto->setResolution(r);
}

void MainWindow::on_spinSizeY_valueChanged(int arg1)
{ // rows count (resolution)
    QSize r = cellauto->resolution();
    r.setHeight(arg1);
    if (!isnan(proportion))
        r.setWidth(int(arg1*proportion)); // W/H = w/h
    cellauto->setResolution(r);
}

void MainWindow::on_editProjectName_textChanged(const QString &arg1)
{ // project name
    modified = true;
}

void MainWindow::on_propButton_toggled(bool checked)
{ // make proportional resolution
    if (checked){
        QSize r = cellauto->resolution();
        proportion = r.width()/r.height();
    }
    else
        proportion = NAN;
}


void MainWindow::on_zoomInBtn_pressed()
{
    moveZoomSlider(1);
}

void MainWindow::on_zoomOutBtn_pressed()
{
    moveZoomSlider(-1);
}


void MainWindow::on_actionSave_triggered()
{
    saveToSomeFile();
}


void MainWindow::on_actionOpen_triggered()
{
    openSomeFile();
}


void MainWindow::on_actionMove_triggered(bool checked)
{
    if (checked)
        cellauto->setMode(CA_PAN);
}


void MainWindow::on_actionDraw_triggered(bool checked)
{
    if (checked){
        cellauto->setMode(CA_DRAW);
        cellauto->setPenState(selectedState);
    }
}

void MainWindow::on_actionErase_triggered(bool checked)
{
    if (checked){
        cellauto->setMode(CA_DRAW);
        cellauto->setPenState(0);
    }
}

void MainWindow::on_actionResume_triggered()
{
    cellauto->setTransitionFunction(ui->plainTextEdit->toPlainText());
    cellauto->resumeAutomaton();
    ui->actionPause->setEnabled(true);
    ui->actionResume->setEnabled(false);
    //# write running status
}

void MainWindow::on_actionPause_triggered()
{
    cellauto->pauseAutomaton();
    ui->actionPause->setEnabled(false);
    ui->actionResume->setEnabled(true);
    //# write paused status
}

bool MainWindow::projectClosed()
{
    QMessageBox::StandardButton btn;
    btn = QMessageBox::question(this, "Question", "Are you sure to close this project?",
                                QMessageBox::Yes | QMessageBox::No |
                                 QMessageBox::Cancel, QMessageBox::Yes);
    if ((btn != QMessageBox::Yes) && (btn != QMessageBox::No))
        return false; // canceled

    if (btn == QMessageBox::Yes){
        saveToSomeFile();
    }

    ui->editProjectName->setText("");
    //# transition to empty or initial
    ui->spinStep->setValue(1);
    ui->spinInterval->setValue(500);
    ui->spinSizeX->setValue(100);
    ui->spinSizeY->setValue(100);
    colorListWidget->clearStates(true);
    cellauto->setStateColor(0, Qt::white);
    cellauto->setStateColor(1, Qt::black);
    colorListWidget->insertStateItem(0);
    colorListWidget->insertStateItem(1);
    zoomTo(1);
    //# status coord icon
    statusCellCoord->setText("0, 0");
    projectModified(false);
    cellauto->clearAutomaton();

    return true;
}

void MainWindow::saveToSomeFile(bool saveas)
{
    QString filename;
    if ((currentfile == "") || saveas){
        filename = QFileDialog::getSaveFileName(this, tr("Save file"),
                                    "", "*.json");
    }
    else
        filename = currentfile;
    if (cellauto->saveToFile(filename))
        currentfile = filename;
}

void MainWindow::openSomeFile()
{
    QString filename;
    if (projectClosed()){
        filename = QFileDialog::getOpenFileName(this, tr("Open file"), "", "*.json");
        if (cellauto->loadFromFile(filename))
            currentfile = filename;
    }
}

void MainWindow::projectModified(bool yes)
{

}

// zooming

bool modifyingzoom = false;

void MainWindow::zoomTo(double z)
{
    if (modifyingzoom) return;
    modifyingzoom = true;

    cellauto->scaleTo(z);
    ui->zoomSlider->setValue(sliderFromZoom(z));
    ui->zoomCombo->setCurrentText(zoomString(z));

    modifyingzoom = false;
}

void MainWindow::moveZoomSlider(int dir)
{
    if (dir == 0) return;
    int delta = 1000*dir;

    int v = ui->zoomSlider->value() + delta;
    int m = v % 1000;
    if (dir == 1)
        v = v - m;
    else
        v = (m == 0)? v : v + (1000 - m);

    ui->zoomSlider->setValue(v);
}

void MainWindow::on_zoomSlider_valueChanged(int value)
{
    if (modifyingzoom) return;
    zoomTo(zoomFromSlider(value));
}

void MainWindow::on_zoomCombo_currentTextChanged(const QString &arg1)
{
    if (modifyingzoom) return;
    double z;
    if (scanZoomString(arg1, z))
        zoomTo(z);
}

void MainWindow::on_cellauto_scaled(double z)
{
    if (modifyingzoom) return;
    zoomTo(z);
}

double MainWindow::zoomFromSlider(int v)
{
    double z;
    if (v == 10000) z = 1;
    else if (v < 10000) z = 0.0001*v;
    else z = 1 + 0.001*(v - 10000);
    return z;
}

int MainWindow::sliderFromZoom(double z)
{
    int v;
    if (z == 1) v = 10000;
    else if (z < 1) v = int(z*10000);
    else v = int((z - 1)*1000 + 10000);
    return v;
}

QString MainWindow::zoomString(double z)
{
    QString s;
    s = QString::number(std::trunc(z*10000.0)/100.0);
    return(s + "%");
}

bool MainWindow::scanZoomString(QString s, double &z)
{
    s = s.trimmed();
    if (s == tr("Reset")){
        z = 1;
        return true;
    }

    if (s[s.count() - 1] == "%")
        s = s.remove(s.count() - 1, 1);
    bool ok;
    z = s.toDouble(&ok)/100;
    return ok;
}

// ---

void MainWindow::on_actionNew_triggered()
{
    if (projectClosed()){}
}


void MainWindow::on_actionClose_triggered()
{
    if (projectClosed()){}
}


void MainWindow::on_colorListWidget_itemClicked(QListWidgetItem *item)
{
    if (item == addColorItem){
        int state = colorListWidget->statesCount();
        if (state > 255) return;
        // open color dialog
        QColor newcolor;
        newcolor = QColorDialog::getColor(cellauto->stateColor(state), this,
                               "Select color");
        newcolor.setAlpha(0);
        cellauto->setStateColor(state, newcolor);
        // create item
        colorListWidget->insertStateItem(state);
        selectedState = state;
        cellauto->setPenState(state);
    }
    else if (item == delColorItem){
        int state = colorListWidget->statesCount() - 1;
        colorListWidget->deleteStateItem(state);
    }
}

void MainWindow::on_colorListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if ((item != addColorItem) && (item != delColorItem)){
        int state = colorListWidget->row(item);
        if (state > 255) return;
        QColor newcolor;
        newcolor = QColorDialog::getColor(cellauto->stateColor(state), this,
                               "Select color");
        newcolor.setAlpha(0);
        cellauto->setStateColor(state, newcolor);
        cellauto->repaint();
    }
}

void MainWindow::on_colorListWidget_itemSelectionChanged()
{
    int currentrow = colorListWidget->currentRow();
    if ((currentrow >= 0) && (currentrow < colorListWidget->statesCount()))
        selectedState = currentrow;
    else
        selectedState = 0;
    cellauto->setPenState(selectedState);
}

void MainWindow::on_cellauto_readCAMetadata(QJsonObject *metadata)
{
    QJsonValue projectname = metadata->value("project");
    QJsonValue states = metadata->value("states");

    if (projectname.isString())
        ui->editProjectName->setText(projectname.toString());
    if (states.isDouble()){
        int statecount = states.toInt();
        if (statecount > 255) statecount = 255;
        colorListWidget->clearStates(false);
        for (int i = 0; i < statecount; i++)
            colorListWidget->insertStateItem(0);
    }
}

void MainWindow::on_cellauto_writeCAMetadata(QJsonObject *metadata)
{
    metadata->insert("project", ui->editProjectName->text());
    metadata->insert("states", colorListWidget->statesCount());
}

void MainWindow::on_actionZoomIn_triggered()
{
    moveZoomSlider(1);
}


void MainWindow::on_actionZoomOut_triggered()
{
    moveZoomSlider(-1);
}


void MainWindow::on_actionZoom100_triggered()
{
    zoomTo(1);
}


void MainWindow::on_actionExit_triggered()
{

}


void MainWindow::on_actionClear_triggered()
{
    cellauto->clearAutomaton();
    projectModified();
}


void MainWindow::on_actionAbout_triggered()
{
    aboutdlg->show();
    aboutdlg->raise();
}

