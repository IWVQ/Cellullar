#ifndef CELLULARWIDGET_H
#define CELLULARWIDGET_H

#include <QtWidgets>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include "cellularautomaton.h"

#define CA_NONE 0
#define CA_DRAW 1
#define CA_PAN 2

class CellularThread: public QThread
{
    Q_OBJECT
public:
    CellularThread(QObject *parent = 0);
    ~CellularThread();

    void run() override;
    void pause();
    void resume();
    void render();

    CellularAutomaton *automaton; // ref
    int step;
    int interval; // sleep interval
    bool repose;
    bool paused;
    QMutex mutex;
    QWaitCondition condition;
signals:
    void rendered(const QImage &image);
private:
    bool abort;
};

class CellularWidgetPrivate;

class Q_DECL_EXPORT CellularWidget: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double scale READ scale WRITE setScale)
    Q_PROPERTY(int interval READ interval WRITE setInterval)
    Q_PROPERTY(int step READ step WRITE setStep)
    Q_PROPERTY(QSize resolution READ resolution WRITE setResolution)
    Q_PROPERTY(char penState READ penState WRITE setPenState)
    Q_PROPERTY(QColor backColor READ backColor WRITE setBackColor)

public:
    enum BinaryState {
        Dead = 0,
        Live = 1
    };
    Q_ENUM(BinaryState)

    explicit CellularWidget(QWidget* parent = nullptr);
    ~CellularWidget() override;

    void resumeAutomaton();
    void pauseAutomaton();
    void resizeAutomaton(int rows, int cols);
    void clearAutomaton();

    bool saveToFile(QString s);
    bool loadFromFile(QString s);

    void setStep(int s);
    void setScale(double s);
    void setInterval(int i);
    void setResolution(QSize r);
    void setPenState(char c);
    void setCell(int i, int j, char c); // 1-indexed
    void setStateColor(char s, QColor c);
    void setMode(int m);
    void setTransitionFunction(QString s);
    void setBackColor(QColor c);
    int step();
    double scale();
    int interval();
    QSize resolution();
    char penState();
    char cell(int i, int j); // 1-indexed
    QColor stateColor(char s);
    int mode();
    QString transitionFunction();
    QColor backColor();

    void cellFromPoint(QPoint p, int &i, int &j); // 1-indexed

    void scaleTo(double s);
signals:
    void readCAMetadata(QJsonObject *metadata);
    void writeCAMetadata(QJsonObject *metadata);
    void scaled(double s);
public slots:
    void rendered(const QImage &image);
protected:
    void zoom(QPoint p, double factor);
    void scroll(QPoint delta);
    QPoint imagePixelAt(QPoint p);
    void refresh(bool rep = false);
    void boundScale();
    void boundOffset();
protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
private:
    QImage image;
    CellularThread cellthread;
    CellularAutomaton automaton;
    std::unique_ptr<class CellularWidgetPrivate> const d;
};

#endif // CELLULARWIDGET_H
