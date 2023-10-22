#ifndef CELLULARAUTOMATON_H
#define CELLULARAUTOMATON_H

#include <QtWidgets>
#include <QJSEngine>

class CellularAutomaton
{
public:

    CellularAutomaton()
    {
        curr = nullptr;
        next = nullptr;
        abort = false;
        n = 10;
        m = 10;
        // colors
        for (int c = 0; c < 256; c++){
            colors[c] = 0xffffff;
        }
        colors[1] = 0x000000;
        // init
        init();
    }

    ~CellularAutomaton()
    {
        release();
    }

    char get(int i, int j)
    {   // 1-indexed
        if ((1 <= i) && (i <= n) && (1 <= j) && (j <= m)){
            return curr[i][j];
        }
        return 0;
    }

    bool edit(int i, int j, char c)
    {   // 1-indexed
        if ((1 <= i) && (i <= n) && (1 <= j) && (j <= m)){
            curr[i][j] = c;
            next[i][j] = c;
            return true;
        }
        return false;
    }

    void resize(int dn /* rows */, int dm /* cols */)
    {
        if ((dn == n) || (dm == m))
            return;
        // backup
        for(int i = 1; i <= n; i++){ // row
            for(int j = 1; j <= m; j++){ // col
                next[i][j] = curr[i][j];
            }
        }
        // deleting curr
        for (int i = 0; i < n + 2; i++)
            delete[] curr[i];
        delete[] curr;
        // creating curr
        curr = new char*[dn + 2];
        for (int i = 0; i < dn + 2; i++) {
            curr[i] = new char[dm + 2];
        }
        // init curr
        for(int i = 0; i < dn + 2; i++){ // row
            for(int j = 0; j < dm + 2; j++){ // col
                curr[i][j] = 0;
            }
        }
        // recovering
        for(int i = 1; i <= n; i++){ // row
            for(int j = 1; j <= m; j++){ // col
                curr[i][j] = next[i][j];
            }
        }
        // deleting next
        for (int i = 0; i < n + 2; i++)
            delete[] next[i];
        delete[] next;
        // creating next
        next = new char*[dn + 2];
        for (int i = 0; i < dn + 2; i++) {
            next[i] = new char[dm + 2];
        }
        // init next
        for(int i = 0; i < dn + 2; i++){ // row
            for(int j = 0; j < dm + 2; j++){ // col
                next[i][j] = 0;
            }
        }
        // new size
        n = dn;
        m = dm;
    }

    void release()
    {
        // deleting curr
        for (int i = 0; i < n + 2; i++)
            delete[] curr[i];
        delete[] curr;
        // deleting next
        for (int i = 0; i < n + 2; i++)
            delete[] next[i];
        delete[] next;
        // make null
        curr = nullptr;
        next = nullptr;
    }

    void init()
    {
        // creating
        curr = new char*[n + 2];
        for (int i = 0; i < n + 2; i++) {
            curr[i] = new char[m + 2];
        }
        next = new char*[n + 2];
        for (int i = 0; i < n + 2; i++) {
            next[i] = new char[m + 2];
        }
        // init
        for(int i = 0; i < n + 2; i++){ // row
            for(int j = 0; j < m + 2; j++){ // col
                curr[i][j] = 0;
                next[i][j] = 0;
            }
        }
    }

    void clear()
    {
        for(int i = 1; i <= n; i++){ // row
            for(int j = 1; j <= m; j++){ // col
                curr[i][j] = 0;
            }
        }
    }

    void interrupt()
    {
        abort = true;
    }

    void resume()
    {
        abort = false;
    }

    void evolve(int s)
    {
        while ((s > 0) && !abort){
            // next generation
            char o;
            for(int i = 1; i <= n; i++){ // row
                if (abort) break;
                for(int j = 1; j <= m; j++){ // col
                    if (abort) break;
                    next[i][j] = transition(i, j, o);
                    if (o != curr[i][j])
                        next[i][j] = curr[i][j]; // edition ovewrites transition
                }
            }
            if (abort) break;

            // exchange
            mutex->lock();
            char **aux;
            aux = curr;
            curr = next; // most efficient way
            next = aux;
            mutex->unlock();

            s--;
        }
    }

    char transition(int i, int j, char &o)
    {
             o = curr[i + 0][j + 0];
        char a = curr[i + 1][j + 0];
        char b = curr[i + 1][j + 1];
        char c = curr[i + 0][j + 1];
        char d = curr[i - 1][j + 1];
        char e = curr[i - 1][j + 0];
        char f = curr[i - 1][j - 1];
        char g = curr[i + 0][j - 1];
        char h = curr[i + 1][j - 1];

        // Conway's game of life

        int s = a + b + c + d + e + f + g + h;
        if (o == 1){
            if (s < 2) return 0;
            else if (s > 3) return 0;
            else return 1;
        }
        else if (s == 3) return 1;
        else return 0;

        //
        return call(o, a, b, c, d, e, f, g, h);
    }

    void render(QImage &image)
    {
        int x, y;
        for (int i = 1; i <= n; i++){
            y = i - 1;
            uint *scanline = reinterpret_cast<uint *>(image.scanLine(y));
            for (int j = 1; j <= m; j++){
                x = j - 1;
                *scanline++ = colors[static_cast<int>(curr[i][j])];
            }
        }
    }

    QSize resolution()
    {
        return QSize{m, n};
    }

    bool readFromJSON(QJsonObject &json)
    {
        // format key
        QJsonValue v = json["caformat"];
        if (!v.isDouble())
            return false;

        // resolution
        release();

        v = json["height"];
        n = v.toInt();
        v = json["width"];
        m = v.toInt();
        if (n < 1) n = 1;
        if (m < 1) m = 1;

        // colors
        QJsonArray jcolors;
        v = json["colors"];
        if (v.isArray()){
            jcolors = v.toArray();
            int i;
            for (i = 0; i < jcolors.count(); i++){
                if (i == 256) break;
                v = jcolors[i];
                if (v.isString()){
                    bool ok = false;
                    colors[i] = v.toString().toUInt(&ok, 16);
                }
            }
            while (i < 256){
                colors[i] = colors[0];
                i++;
            }
        }

        // transition
        QJsonArray jtransition;
        v = json["transition"];
        QString s("");
        if (v.isArray()){
            jtransition = v.toArray();
            for (int i = 0; i < jtransition.count(); i++){
                s.append(jtransition[i].toString());
            }
        }
        compile(s);

        // data
        QByteArray encodeddata;
        QString datastr = json["data"].toString();
        encodeddata.append(datastr);
        QByteArray data = QByteArray::fromBase64(encodeddata);
        init();
        int l = data.count();
        int k = 0;
        for (int i = 1; i <= n; i++){
            for (int j = 1; j <= m; j++){
                k = (i - 1)*m + (j - 1);
                if (k >= l) break;
                curr[i][j] = data[k];
            }
            if (k >= l) break;
        }

        return true;
    }

    bool writeToJSON(QJsonObject &json)
    {
        // resolution
        json["width"] = m;
        json["height"] = n;
        // colors
        int l = 255;
        while (l > 0){
            if (colors[0] != colors[l])
                break;
            l--;
        }
        QJsonArray jcolors;
        for (int i = 0; i <= l; i++){
            QString hex = QString("%1").arg(colors[i], 6, 16, QLatin1Char('0'));
            jcolors.append(hex);
        }
        json["colors"] = jcolors;

        // transition rule
        QJsonArray jtransition;
        QStringList strlist = source.split("\n");
        QString str;
        for (int i = 0; i < strlist.count(); i++){
            str = strlist.at(i);
            str.append("\n");
            jtransition.append(str);
        }
        json["transition"] = jtransition;

        // cell data
        QByteArray data;
        for (int i = 0; i <= n; i++){
            data.append(QByteArray(curr[i], m));
        }
        json["data"] = QString(data.toBase64());

        // format key
        json["caformat"] = 1;

        return true;
    }

    void compile(QString s)
    {
        source = s;
        s.insert(0, '(');
        s.append(')');
        function = engine.evaluate(s);
    }

    char call(char o, char a, char b, char c, char d, char e, char f, char g, char h)
    {
        QJSValue v(0);
        QJSValueList args;
        args << o << a << b << c << d << e << f << g << h;
        if (function.isCallable())
            v = function.call(args);
        return v.toInt();
    }

    uint colors[256];
    int n, m; // real dimensions
    QString source;
    QJSValue function;
    QJSEngine engine;
    QMutex *mutex;
private:
    bool abort;
    char** curr; // n+2 X m+2
    char** next;
};

#endif // CELLULARAUTOMATON_H
