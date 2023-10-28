#include "cellularwidget.h"

#include <stdio.h>
#include <string.h>

#include <QtGlobal>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

const double ZoomFactor = 0.8f;

/* CellularThread */

CellularThread::CellularThread(QObject *parent): QThread(parent)
{
    abort = false;
    paused = false;
    repose = true;
    step = 1;
    interval = 500;
    automaton = nullptr;
}

CellularThread::~CellularThread()
{
    mutex.lock();
    abort = true;
    repose = false;
    if (automaton)
        automaton->interrupt();
    condition.wakeAll();
    mutex.unlock();
    wait();
}

void CellularThread::run()
{
    while(!abort){
        if (sourcemodified){
            mutex.lock();
            automaton->compile(source);
            sourcemodified = false;
            mutex.unlock();
        }

        automaton->evolve(step);
        render(); // necesary for current state
        if (abort) break;
        if (paused){
            const QMutexLocker locker(&mutex);
            repose = true;
            condition.wait(&mutex);
            automaton->resume();
            paused = false;
        }
        else if (interval > 0){
            msleep(interval);
        }
    }
}

void CellularThread::pause()
{
    const QMutexLocker locker(&mutex);
    if (isRunning() && !paused){
        paused = true;
        automaton->interrupt();
    }
}

void CellularThread::resume()
{
    const QMutexLocker locker(&mutex);
    repose = false;
    if (isRunning()){
        condition.wakeAll();
    }
    else
        start(LowPriority);
}

void CellularThread::render()
{
    QImage image(automaton->resolution(), QImage::Format_RGB32);
    automaton->render(image);
    emit rendered(image);
}

void CellularThread::compile(QString s)
{
    if (repose){
        source = s;
        sourcemodified = true;
    }
}

/* CellHistory */

CellHistory::CellHistory()
{
    init();
}

CellHistory::~CellHistory()
{
    delete [] actions;
}

void CellHistory::init()
{
    groupdepth = 0;
    size = GROW_SIZE;
    actions = new Action[size];
    current = 0;
    count = 0;
    actions[current].start = true;
}

void CellHistory::ensureRoom()
{
    if (count + 1 == size){
        if (size == MAX_ACTION_COUNT){
            // move back by GROW_SIZE
            memmove(actions, actions + GROW_SIZE, sizeof(Action)*(MAX_ACTION_COUNT - GROW_SIZE));
            count -= GROW_SIZE;
            current -= GROW_SIZE;
        }
        else{
            size += GROW_SIZE;
            if (size > MAX_ACTION_COUNT) size = MAX_ACTION_COUNT;
            Action *newactions = new Action[size];
            // copy from
            memcpy(newactions, actions, (count + 1) * sizeof(Action));
            delete [] actions;
            actions = newactions;
            newactions = nullptr;
        }
    }
}

void CellHistory::beginGroup()
{
    if (groupdepth == 0)
        actions[current].start = true;
    groupdepth++;
}

void CellHistory::append(int i, int j, char be, char af)
{
    ensureRoom();
    //
    actions[current].i = i;
    actions[current].j = j;
    actions[current].before = be;
    actions[current].after = af;
    current++;
    actions[current].start = false;
    count = current; // new count
}

void CellHistory::endGroup()
{
    groupdepth--;
    if (groupdepth == 0)
        actions[current].start = true;
}

void CellHistory::clear()
{
    delete [] actions;
    init();
}

bool CellHistory::canUndo()
{
    return current > 0;
}

bool CellHistory::canRedo()
{
    return current < count;
}

CellHistory::Action CellHistory::undoStep()
{
    current--;
    return actions[current];
}

CellHistory::Action CellHistory::redoStep()
{
    return actions[current];
    current ++;
}

bool CellHistory::stopUndo()
{
    return actions[current].start || (current == 0);
}

bool CellHistory::stopRedo()
{
    return actions[current].start || (current == count);
}

/* CellularWidgetPrivate */

class CellularWidgetPrivate
{
public:
    CellularWidgetPrivate(CellularWidget *qq);
    CellularWidget *q;

    int interval;
    int step;
    char penstate;

    double scale;
    QPoint anchor;
    QPoint offset;
    QPoint center;
    QColor backcolor;

    int mode;
    CellHistory history;
};

CellularWidgetPrivate::CellularWidgetPrivate(CellularWidget *qq):q(qq)
{
    interval = 500; // milliseconds
    step = 1;
    penstate = 0;

    scale = 1;
    anchor = QPoint();
    offset = QPoint{0, 0};
    center = QPoint(0, 0);
    backcolor = Qt::black;

    mode = CA_NONE;
}

/* CellularWidget */

CellularWidget::CellularWidget(QWidget* parent):
    QWidget(parent),
    d(new CellularWidgetPrivate(this))
{
    cellthread.automaton = &automaton;
    automaton.mutex = &(cellthread.mutex);
    connect(&cellthread, SIGNAL(rendered(QImage)), this, SLOT(rendered(QImage)));
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);
}

CellularWidget::~CellularWidget() = default;

void CellularWidget::resumeAutomaton()
{
    clearHistory();
    emit modified();
    cellthread.resume();
}

void CellularWidget::pauseAutomaton()
{
    cellthread.pause();
}

void CellularWidget::resizeAutomaton(int rows, int cols)
{
    if ((rows <= 0) || (cols <= 0))
        return;
    if (cellthread.repose){ // resize only if in repose
        if ((automaton.m != cols) || (automaton.n != rows)){
            automaton.resize(rows, cols);
            refresh();
            emit modified();
        }
    }
}

void CellularWidget::clearAutomaton()
{
    if (cellthread.repose){
        automaton.clear();
        clearHistory(); //#
        refresh();
        emit modified();
    }
}

bool CellularWidget::saveToFile(QString s)
{
    if (cellthread.repose){
        QJsonObject json;

        QJsonObject metadata;
        metadata["scale"] = d->scale;
        emit writeCAMetadata(&metadata);
        json["metadata"] = metadata;
        automaton.writeToJSON(json);

        QFile savefile(s);
        if (!savefile.open(QIODevice::WriteOnly)) {
            qWarning("Couldn't open automaton file.");
            return false;
        }
        savefile.write(QJsonDocument(json).toJson());
        return true;
    }
    return false;
}

bool CellularWidget::loadFromFile(QString s)
{
    if (cellthread.repose){
        QFile loadfile(s);
        if (!loadfile.open(QIODevice::ReadOnly)) {
            qWarning("Couldn't open automaton file.");
            return false;
        }
        QByteArray data = loadfile.readAll();
        QJsonDocument jdoc(QJsonDocument::fromJson(data));

        QJsonObject json = jdoc.object();

        if (automaton.readFromJSON(json)){
            clearHistory();
            QJsonValue v = json["metadata"];
            if (v.isObject()){
                QJsonObject metadata = v.toObject();
                v = metadata["scale"];
                if (v.isDouble())
                    setScale(v.toDouble());
                emit readCAMetadata(&metadata);
            }
            refresh();
            return true;
        }
    }
    return false;
}

void CellularWidget::setStep(int s)
{
    if (s > 0){
        d->step = s;
        cellthread.step = d->step;
    }
}

void CellularWidget::setScale(double s)
{
    if ((s > 0) && (d->scale != s)){
        d->scale = s;
        refresh();
        emit scaled(d->scale);
    }
}

void CellularWidget::setInterval(int i)
{
    if (i >= 0){
        d->interval = i;
        cellthread.interval = d->interval;
    }
}

void CellularWidget::setResolution(QSize r)
{
    resizeAutomaton(r.height(), r.width());
}

void CellularWidget::setPenState(char c)
{
    if (d->penstate != c){
        d->penstate = c;
    }
}

void CellularWidget::setCell(int i, int j, char c)
{
    char o;
    if (internalEdit(i, j, c, o)){
        d->history.append(i, j, o, c);
        emit modified();
    }
}

void CellularWidget::setStateColor(char s, QColor c)
{
    if (automaton.colors[static_cast<int>(s)] != c.rgb()){
        uint cl = c.rgb();
        cl &= 0x00ffffff;
        automaton.colors[static_cast<int>(s)] = cl;
        refresh();
        emit modified();
    }
}

void CellularWidget::setMode(int m)
{
    d->mode = m;
}

void CellularWidget::setTransitionFunction(QString s)
{
    if ((cellthread.repose) && (s != cellthread.source)){
        cellthread.compile(s);
        emit modified();
    }
}

void CellularWidget::setBackColor(QColor c)
{
    if (d->backcolor != c){
        d->backcolor = c;
        update();
    }
}

void CellularWidget::setEvolCounter(int c)
{
    automaton.setCounter(c);
}

int CellularWidget::step()
{
    return d->step;
}

double CellularWidget::scale()
{
    return d->scale;
}

int CellularWidget::interval()
{
    return d->interval;
}

QSize CellularWidget::resolution()
{
    return automaton.resolution();
}

char CellularWidget::penState()
{
    return d->penstate;
}

char CellularWidget::cell(int i, int j)
{
    return automaton.get(i, j);
}

QColor CellularWidget::stateColor(char s)
{
    return automaton.colors[static_cast<int>(s)];
}

int CellularWidget::mode()
{
    return d->mode;
}

QString CellularWidget::transitionFunction()
{
    return cellthread.source;
}

QColor CellularWidget::backColor()
{
    return d->backcolor;
}

int CellularWidget::evolCounter()
{
    return automaton.counter;
}

void CellularWidget::cellFromPoint(QPoint p, int &i, int &j)
{
    QPoint q = imagePixelAt(p);
    i = q.y() + 1; // row
    j = q.x() + 1; // col
}

void CellularWidget::modify()
{
    emit modified();
}


void CellularWidget::undo()
{
    if (canUndo()){
        char o;
        CellHistory::Action action;
        do{
            action = d->history.undoStep();
            if (internalEdit(action.i, action.j, action.before, o))
                emit modified();
        }
        while (!d->history.stopUndo());
    }
}

void CellularWidget::redo()
{
    if (canRedo()){
        char o;
        CellHistory::Action action;
        do{
            action = d->history.redoStep();
            if (internalEdit(action.i, action.j, action.after, o))
                emit modified();
        }
        while (!d->history.stopRedo());
    }
}

bool CellularWidget::canUndo()
{
    return (cellthread.repose && d->history.canUndo());
}

bool CellularWidget::canRedo()
{
    return (cellthread.repose && d->history.canRedo());
}

void CellularWidget::clearHistory()
{
    d->history.clear();
}

void CellularWidget::refresh(bool rep)
{
    QImage img(automaton.resolution(), QImage::Format_RGB32);
    automaton.render(img);
    image = img;

    boundScale();
    boundOffset();

    if (rep)
        repaint();
    else
        update();
}

void CellularWidget::scaleTo(double s)
{
    if (s == d->scale) return;

    QPoint p = d->center;
    if (s < 0.1) s = 0.1;
    if (s > 10) s = 10;
    // f = s/scale
    d->offset = p - (p - d->offset)*s/d->scale;
    d->scale = s;
    // correction
    boundScale();
    boundOffset();
    update();
    // signal
    emit scaled(d->scale);
}

void CellularWidget::zoom(QPoint p, double factor)
{
    // zooming
    if (d->scale*factor < 0.1) factor = 0.1/d->scale;
    if (d->scale*factor > 10) factor = 10/d->scale;

    d->offset = p - (p - d->offset)*factor;
    d->scale *= factor;

    // correction
    boundScale();
    boundOffset();
    update();

    // signal
    emit scaled(d->scale);
}

void CellularWidget::scroll(QPoint delta)
{
    // panning
    d->offset += delta;

    // correction
    boundOffset();
    update();
}

void CellularWidget::boundScale()
{
    if (d->scale < 0.1) d->scale = 0.1;
    if (d->scale > 10) d->scale = 10;
}

void CellularWidget::boundOffset()
{
    int mxx = d->center.x();
    int mxy = d->center.y();
    int mnx = d->center.x() - int(image.width()*d->scale);
    int mny = d->center.y() - int(image.height()*d->scale);
    if (d->offset.x() < mnx) d->offset.setX(mnx);
    if (d->offset.y() < mny) d->offset.setY(mny);
    if (d->offset.x() > mxx) d->offset.setX(mxx);
    if (d->offset.y() > mxy) d->offset.setY(mxy);
}

QPoint CellularWidget::imagePixelAt(QPoint p)
{
    return (p - d->offset)/d->scale;
}

bool CellularWidget::internalEdit(int i, int j, char c, char &old)
{
    if (automaton.edit(i, j, c)){
        image.setPixel(j - 1, i - 1, automaton.colors[static_cast<int>(c)]);
        repaint();
        return true;
    }
    return false;
}

void CellularWidget::rendered(const QImage &image)
{
    cellthread.mutex.lock();
    this->image = image;
    cellthread.mutex.unlock();
    repaint();
    emit evolved();
}

void CellularWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.fillRect(rect(), d->backcolor);

    if (d->scale == 1) {
        painter.drawImage(d->offset, image);
    } else {
        painter.save();
        painter.translate(d->offset.x(), d->offset.y());
        painter.scale(d->scale, d->scale);
        QRectF exposed = painter.matrix().inverted().mapRect(rect()).adjusted(-1, -1, 1, 1);
        painter.drawImage(exposed, image, exposed);
        painter.restore();
    }
}

void CellularWidget::resizeEvent(QResizeEvent *event)
{
    d->center = rect().center();
    refresh();
    QWidget::resizeEvent(event);
}

void CellularWidget::mousePressEvent(QMouseEvent *event)
{
    int i, j;
    QPoint p = event->pos();
    switch (d->mode){
    case CA_PAN:
        d->anchor = QPoint();
        if ((Qt::LeftButton & event->buttons()) == Qt::LeftButton){
            d->anchor = p;
        }
        break;
    case CA_DRAW:
        cellFromPoint(p, i, j);
        if ((Qt::LeftButton & event->buttons()) == Qt::LeftButton){
            d->history.beginGroup();
            setCell(i, j, d->penstate);
        }
        break;
    }
    QWidget::mousePressEvent(event);
}

void CellularWidget::mouseMoveEvent(QMouseEvent *event)
{
    int i, j;
    QPoint p = event->pos();
    if ((Qt::LeftButton & event->buttons()) == Qt::LeftButton){
        switch (d->mode){
        case CA_PAN:
            if ((Qt::LeftButton & event->buttons()) == Qt::LeftButton){
                scroll(p - d->anchor);
                d->anchor = p;
            }
            break;
        case CA_DRAW:
            cellFromPoint(p, i, j);
            if ((Qt::LeftButton & event->buttons()) == Qt::LeftButton){
                setCell(i, j, d->penstate);
            }
            break;
        }
    }
    emit planning(p);
    QWidget::mouseMoveEvent(event);
}

void CellularWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    if (d->mode == CA_DRAW){
        if ((Qt::LeftButton & event->buttons()) == Qt::LeftButton)
            d->history.endGroup();
    }
    else if (d->mode == CA_PAN){
        if (event->button() == Qt::LeftButton) {
            scroll(p - d->anchor);
            d->anchor = QPoint();
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void CellularWidget::leaveEvent(QEvent *event)
{
    emit planning(QPoint{16777215, 16777215});
    QWidget::leaveEvent(event);
}

void CellularWidget::wheelEvent(QWheelEvent *event)
{
    QPoint p = event->pos();
    int numDegrees = event->delta()/8;
    if (Qt::CTRL & event->modifiers()){
        // zoom
        double numSteps = - numDegrees/15.0f;
        zoom(p, pow(ZoomFactor, numSteps));
    }
    else if (Qt::SHIFT & event->modifiers()){
        // horizontal pan
        scroll(QPoint{int(d->scale*numDegrees), 0});
    }
    else {
        // vertical pan
        scroll(QPoint{0, int(d->scale*numDegrees)});
    }
    QWidget::wheelEvent(event);
}
