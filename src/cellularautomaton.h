#ifndef CELLULARAUTOMATON_H
#define CELLULARAUTOMATON_H

#include <QtWidgets>
#include <QJSEngine>

class CellularAutomaton
{
public:

    // creation

    CellularAutomaton();
    ~CellularAutomaton();
    void release();
    void init();

    // edition

    char get(int i, int j);
    bool edit(int i, int j, char c);
    void resize(int dn /* rows */, int dm /* cols */);
    QSize resolution();
    void clear();

    // evolving

    void interrupt();
    void resume();
    void setCounter(int c);

    void evolve(int s); // thread
    char transition(int i, int j, char &o); // thread
    void compile(QString s); // thread
    char call(char o, char a, char b, char c, char d, char e, char f, char g, char h); // thread

    // rendering

    void render(QImage &image);
    bool readFromJSON(QJsonObject &json);
    bool writeToJSON(QJsonObject &json);

    int counter = 0;
    uint colors[256];
    int n, m; // real dimensions
    QString source;
    QJSValue *function = nullptr;
    QJSEngine *engine = nullptr;
    QMutex *mutex;
private:
    bool abort;
    char** curr; // n+2 X m+2
    char** next;
};

#endif // CELLULARAUTOMATON_H
