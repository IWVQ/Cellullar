#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>

// dcb
// eoa  <-- neighborhood identifiers
// fgh

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
        cellauto->modify();
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
    cellauto->modify();
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
    cellauto->modify();
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
    cellauto->setResolution(QSize{100, 100});
    cellauto->setTransitionFunction(defaultTransitionRule);

    connect(cellauto, SIGNAL(scaled(double)), this, SLOT(on_cellauto_scaled(double)));
    connect(cellauto, SIGNAL(readCAMetadata(QJsonObject*)), this, SLOT(on_cellauto_readCAMetadata(QJsonObject*)));
    connect(cellauto, SIGNAL(writeCAMetadata(QJsonObject*)), this, SLOT(on_cellauto_writeCAMetadata(QJsonObject*)));
    connect(cellauto, SIGNAL(planning(QPoint)), this, SLOT(on_cellauto_planning(QPoint)));
    connect(cellauto, SIGNAL(modified()), this, SLOT(on_cellauto_modified()));
    connect(cellauto, SIGNAL(evolved()), this, SLOT(on_cellauto_evolved()));

    // zoom bar
    ui->zoomCombo->lineEdit()->setAlignment(Qt::AlignRight);
    ui->statusbar->addPermanentWidget(ui->zoomWidget);

    // coord status

    coordStatusWdg = new QWidget(ui->statusbar);
    coordStatusWdg->setObjectName(QString::fromUtf8("coordStatusWdg"));
    coordStatusWdg->setVisible(true);
    coordStatusWdg->setMinimumSize(QSize{100, 0});
    coordStatusWdg->setMaximumSize(QSize{100, 16777215});

    coordStatusLayout = new QHBoxLayout(coordStatusWdg);
    coordStatusLayout->setObjectName(QString::fromUtf8("coordStatusLayout"));
    coordStatusLayout->setContentsMargins(9, 0, 0, 0);

    coordStatusImg = new QLabel(coordStatusWdg);
    coordStatusImg->setObjectName(QString::fromUtf8("coordStatusWdg"));
    QSizePolicy coordStatusSizePolicy = coordStatusImg->sizePolicy();
    coordStatusSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    coordStatusImg->setSizePolicy(coordStatusSizePolicy);
    coordStatusImg->setVisible(true);
    coordStatusImg->setPixmap(QPixmap(QString::fromUtf8(":/lucide-icons/move_16.png")));
    coordStatusLayout->addWidget(coordStatusImg);

    coordStatusLbl = new QLabel(coordStatusWdg);
    coordStatusLbl->setObjectName(QString::fromUtf8("coordStatusLbl"));
    coordStatusLbl->setVisible(true);
    coordStatusLbl->setText("");
    coordStatusLayout->addWidget(coordStatusLbl);

    ui->statusbar->addWidget(coordStatusWdg);

    // modified status

    modifiedStatusLbl = new QLabel(ui->statusbar);
    modifiedStatusLbl->setObjectName(QString::fromUtf8("modifiedStatusLbl"));
    modifiedStatusLbl->setVisible(true);
    modifiedStatusLbl->setMinimumSize(QSize{90, 0});
    modifiedStatusLbl->setMaximumSize(QSize{90, 16777215});
    modifiedStatusLbl->setText("");
    ui->statusbar->addWidget(modifiedStatusLbl);

    // edition group

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

    // about dialog

    aboutdlg = new About(this);

    // edit dialog
    editdlg = new EditDlg();
    editdlg->setTransitionRule(defaultTransitionRule);
    connect(editdlg, SIGNAL(buildTransitionRule(QString)), this,
            SLOT(on_editdlg_buildTransitionRule(QString)));
    connect(editdlg, SIGNAL(showConfig()), this, SLOT(on_editdlg_showConfig()));
    connect(editdlg, SIGNAL(showHelp()), this, SLOT(on_editdlg_showHelp()));

    // config dialog

    configdlg = new Config(this);


    // shortcut for zoomout (Ctrl+-)

    ui->actionZoomOut->setShortcut(QApplication::translate("MainWindow", "Ctrl+-", nullptr));



    // new geometry for side panel

    QRect dockgeo = ui->dockWidget->geometry();
    dockgeo.setWidth(160);
    ui->dockWidget->setGeometry(dockgeo);

    // initial states

    colorListWidget->insertStateItem(0);
    colorListWidget->insertStateItem(1);
    setSelectedState(1);

    //
    projectModified(false);

    // config
    createConfig();
}

MainWindow::~MainWindow()
{
    delete editdlg;
    delete ui;
}

// events

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (projectClosed()){
        editdlg->close();
        event->accept();
    }
    else
        event->ignore();
}

// transition rule

void MainWindow::on_editdlg_buildTransitionRule(const QString &s)
{
    cellauto->setTransitionFunction(s);
}

void MainWindow::loadTransitionRule(QString s)
{
    cellauto->setTransitionFunction(s);
    editdlg->setTransitionRule(s);
}

void MainWindow::editTransitionRule()
{
    //
    editdlg->showEditDialog(cellauto->transitionFunction());
}

void MainWindow::on_pushTransition_clicked()
{
    editTransitionRule();
}

//  --

void MainWindow::on_spinStep_valueChanged(int arg1)
{ // step
    cellauto->setStep(arg1);
}

void MainWindow::on_spinInterval_valueChanged(int arg1)
{ // interval
    cellauto->setInterval(arg1);
}

// resolution

bool modifiyingresolution = false;

void MainWindow::loadResolution(QSize r)
{
    if (modifiyingresolution) return;

    modifiyingresolution = true;
    ui->spinSizeX->setValue(r.width());
    ui->spinSizeY->setValue(r.height());
    on_propButton_toggled(ui->propButton->isChecked());
    modifiyingresolution = false;
}

void MainWindow::on_spinSizeX_valueChanged(int arg1)
{ // cols count (resolution)
    if (modifiyingresolution) return;

    modifiyingresolution = true;
    QSize r = cellauto->resolution();
    r.setWidth(arg1);
    if (!isnan(proportion)){
        r.setHeight(int(arg1/proportion)); // W/H = w/h -> H = W/p
        ui->spinSizeY->setValue(r.height());
    }
    cellauto->setResolution(r);
    modifiyingresolution = false;
}

void MainWindow::on_spinSizeY_valueChanged(int arg1)
{ // rows count (resolution)
    if (modifiyingresolution) return;

    modifiyingresolution = true;
    QSize r = cellauto->resolution();
    r.setHeight(arg1);
    if (!isnan(proportion)){
        r.setWidth(int(arg1*proportion)); // W/H = w/h -> W = H*p
        ui->spinSizeX->setValue(r.width());
    }
    cellauto->setResolution(r);
    modifiyingresolution = false;
}

void MainWindow::on_propButton_toggled(bool checked)
{ // make proportional resolution
    if (checked){
        QSize r = cellauto->resolution();
        if (r.height() == 0) proportion = NAN;
        else proportion = r.width()/r.height();
        ui->propButton->setIcon(QIcon(":/img/link_l.png"));
    }
    else{
        proportion = NAN;
        ui->propButton->setIcon(QIcon(":/img/unlink_l.png"));
    }
}



// --

void MainWindow::on_editProjectName_textChanged(const QString &arg1)
{ // project name
    projectModified();
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
    cellauto->setTransitionFunction(editdlg->transitionRule());
    cellauto->resumeAutomaton();
    ui->actionPause->setEnabled(true);
    ui->actionResume->setEnabled(false);
    editdlg->setRunning(true);
}

void MainWindow::on_actionPause_triggered()
{
    cellauto->pauseAutomaton();
    ui->actionPause->setEnabled(false);
    ui->actionResume->setEnabled(true);
    editdlg->setRunning(false);
}

bool MainWindow::projectClosed()
{
    if (modified){
        QMessageBox::StandardButton btn;
        btn = QMessageBox::question(this, "Question", "Do you want to save this project?",
                                    QMessageBox::Yes | QMessageBox::No |
                                     QMessageBox::Cancel, QMessageBox::Yes);
        if ((btn != QMessageBox::Yes) && (btn != QMessageBox::No))
            return false; // canceled

        if (btn == QMessageBox::Yes){
            saveToSomeFile();
        }
    }
    else
        currentfile = "";


    ui->editProjectName->setText("");
    loadTransitionRule(defaultTransitionRule);
    ui->spinStep->setValue(1);
    ui->spinInterval->setValue(500);
    ui->spinSizeX->setValue(100);
    ui->spinSizeY->setValue(100);
    colorListWidget->clearStates(true);
    cellauto->setStateColor(0, Qt::white);
    cellauto->setStateColor(1, Qt::black);
    colorListWidget->insertStateItem(0);
    colorListWidget->insertStateItem(1);
    setSelectedState(1);
    zoomTo(1);
    coordStatusLbl->setText("");
    cellauto->clearAutomaton();

    coordStatusLbl->setText("");
    ui->propButton->setChecked(false);

    projectModified(false);

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
    if (cellauto->saveToFile(filename)){
        currentfile = filename;
        projectModified(false);
    }
}

void MainWindow::openSomeFile()
{
    QString filename;
    if (projectClosed()){
        filename = QFileDialog::getOpenFileName(this, tr("Open file"), "", "*.json");
        if (cellauto->loadFromFile(filename)){
            currentfile = filename;
            projectModified(false);
        }
    }
}

void MainWindow::projectModified(bool yes)
{
    if (yes){
        if (modified) return;
        modifiedStatusLbl->setText("Modified");
        this->setWindowTitle(cellullarname + " - *" + currentfile);
        modified = true;
    }
    else{
        if (!modified) return;
        modifiedStatusLbl->setText("");
        this->setWindowTitle(cellullarname + " - " + currentfile);
        modified = false;
    }
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

// themes

void MainWindow::applyTheme(int th)
{
    //#
}

// ---

void MainWindow::on_actionNew_triggered()
{
    if (projectClosed()){ projectModified(true);}
}


void MainWindow::on_actionClose_triggered()
{
    if (projectClosed()){ projectModified(false);}
}

// states

void MainWindow::setSelectedState(int state)
{
    if ((state >= 0) and (state < colorListWidget->statesCount())){
        colorListWidget->setCurrentRow(state);
        selectedState = state;
        cellauto->setPenState(state);
    }
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
        setSelectedState(state);
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
    QJsonValue counter = metadata->value("counter");

    if (projectname.isString())
        ui->editProjectName->setText(projectname.toString());
    if (states.isDouble()){
        int statecount = states.toInt();
        if (statecount > 255) statecount = 255;
        colorListWidget->clearStates(false);
        for (int i = 0; i < statecount; i++)
            colorListWidget->insertStateItem(0);
        setSelectedState(1);
    }
    ui->counterLbl->display(counter.toInt(0));
    cellauto->setEvolCounter(counter.toInt(0));
    loadTransitionRule(cellauto->transitionFunction());
    loadResolution(cellauto->resolution());
}

void MainWindow::on_cellauto_writeCAMetadata(QJsonObject *metadata)
{
    metadata->insert("project", ui->editProjectName->text());
    metadata->insert("states", colorListWidget->statesCount());
    metadata->insert("counter", cellauto->evolCounter());
}

//

void MainWindow::on_actionZoomIn_triggered()
{
    if (cellauto->hasFocus())
        moveZoomSlider(1);
}


void MainWindow::on_actionZoomOut_triggered()
{
    if (cellauto->hasFocus())
        moveZoomSlider(-1);
}


void MainWindow::on_actionZoom100_triggered()
{
    if (cellauto->hasFocus())
        zoomTo(1);
}


void MainWindow::on_actionExit_triggered()
{
    close();
}


void MainWindow::on_actionClear_triggered()
{
    cellauto->clearAutomaton();
    projectModified();
}


void MainWindow::on_actionAbout_triggered()
{
    aboutdlg->showAboutDialog();
}

void MainWindow::on_cellauto_planning(QPoint p)
{
    int x, y;
    QSize s = cellauto->resolution();
    cellauto->cellFromPoint(p, y, x);
    if ((1 <= x) && (x <= s.width()) && (1 <= y) && (y <= s.height()))
        coordStatusLbl->setText(QString::number(x) + ", " + QString::number(y));
    else
        coordStatusLbl->setText("");
}

void MainWindow::on_cellauto_modified()
{
    projectModified();
}

void MainWindow::on_cellauto_evolved()
{
    ui->counterLbl->display(cellauto->evolCounter());
}


void MainWindow::on_actionTransitionRule_triggered()
{
    editTransitionRule();
}


void MainWindow::on_counterBtn_clicked()
{
    ui->counterLbl->display(0);
    cellauto->setEvolCounter(0);
}


void MainWindow::on_actionUndo_triggered()
{
    if (cellauto->hasFocus()) { cellauto->undo();}
}


void MainWindow::on_actionRedo_triggered()
{
    if (cellauto->hasFocus()) { cellauto->redo();}
}


void MainWindow::on_actionPrint_triggered()
{ //# print

}


void MainWindow::on_actionDarkTheme_triggered()
{
    applyTheme(DARK_THEME);
}


void MainWindow::on_actionDefaultTheme_triggered()
{
    applyTheme(DEFAULT_THEME);
}

// settings

void MainWindow::createConfig()
{
    QByteArray data;
    QFile loadfile("config.json");
    if (!loadfile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open config file.");
        QFile loadfile("config-default.json");
        data = loadfile.readAll();
    }
    else{
        data = loadfile.readAll();
    }
    config = new QJsonDocument(QJsonDocument::fromJson(data));

    connect(configdlg, SIGNAL(saveConfig(QJsonObject)), this,
            SLOT(on_configdlg_saveConfig(QJsonObject)));

    applyConfig();
}

void MainWindow::applyConfig()
{
    //# apply on me, theme, language
    QJsonObject json = config->object();
    QJsonValue v = json["automaton"];
    if (v.isObject()){
        QJsonObject json_automaton = v.toObject();
        v = json_automaton["counter-digits"];
        ui->counterLbl->setDigitCount(v.toInt(1));
        v = json_automaton["background"];
        bool ok;
        QColor cl = v.toString().toUInt(&ok, 16);
        cellauto->setBackColor(cl);
    }
    //# restart-counter, auto-build
    // apply on dialogs
    aboutdlg->applyConfig(config);
    configdlg->applyConfig(config);
    editdlg->applyConfig(config);
}

void MainWindow::on_configdlg_saveConfig(const QJsonObject &c)
{
    delete config;
    config = new QJsonDocument(c);
    applyConfig();

    QJsonObject json = config->object();
    QFile savefile("config.json");
    if (savefile.open(QIODevice::WriteOnly))
        savefile.write(QJsonDocument(json).toJson());
}

void MainWindow::on_actionPreferences_triggered()
{
    configdlg->showConfig(config);
}


void MainWindow::on_editdlg_showConfig()
{
    configdlg->showConfig(config, Config::Sub_Editor);
}

// help

void MainWindow::on_actionHelp_triggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile("tutorial.md"));
}

void MainWindow::on_editdlg_showHelp()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile("tutorial.md"));
}

