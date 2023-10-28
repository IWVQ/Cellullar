#include "cellularautomaton.h"
#include <cstdlib>
#include <ctime>

//-- FORWARD --//

char conwayGameOfLife(char o, char a, char b, char c, char d, char e, char f, char g, char h);
char rockPaperScissor(char o, char a, char b, char c, char d, char e, char f, char g, char h);
char rockPaperScissorLizardSpock(char o, char a, char b, char c, char d, char e, char f, char g, char h);
char forestFire(char o, char a, char b, char c, char d, char e, char f, char g, char h);
char forestFireWet(char o, char a, char b, char c, char d, char e, char f, char g, char h);
char forestWet(char o, char a, char b, char c, char d, char e, char f, char g, char h);

//-- CREATION --//

CellularAutomaton::CellularAutomaton()
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

    std::srand(std::time(nullptr));
}


CellularAutomaton::~CellularAutomaton()
{
    release();
    if (engine) delete engine;
    if (function) delete function;
}

void CellularAutomaton::release()
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

void CellularAutomaton::init()
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

//-- EDITION --//

char CellularAutomaton::get(int i, int j)
{   // 1-indexed
    if ((1 <= i) && (i <= n) && (1 <= j) && (j <= m)){
        return curr[i][j];
    }
    return 0;
}

bool CellularAutomaton::edit(int i, int j, char c)
{   // 1-indexed
    if ((1 <= i) && (i <= n) && (1 <= j) && (j <= m)){
        curr[i][j] = c;
        next[i][j] = c;
        return true;
    }
    return false;
}

void CellularAutomaton::resize(int dn /* rows */, int dm /* cols */)
{
    if ((dn == n) && (dm == m))
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
    int _n = qMin(n, dn);
    int _m = qMin(m, dm);
    for(int i = 1; i <= _n; i++){ // row
        for(int j = 1; j <= _m; j++){ // col
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

void CellularAutomaton::clear()
{
    for(int i = 1; i <= n; i++){ // row
        for(int j = 1; j <= m; j++){ // col
            curr[i][j] = 0;
        }
    }
}

QSize CellularAutomaton::resolution()
{
    return QSize{m, n};
}

//-- EVOLVING --//

void CellularAutomaton::interrupt()
{
    abort = true;
}

void CellularAutomaton::resume()
{
    abort = false;
}

void CellularAutomaton::evolve(int s)
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
        counter++;
        mutex->unlock();

        s--;
    }
}

char CellularAutomaton::transition(int i, int j, char &o)
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

    //
    //return conwayGameOfLife(o, a, b, c, d, e, f, g, h);
    //return rockPaperScissor(o, a, b, c, d, e, f, g, h);
    //return rockPaperScissorLizardSpock(o, a, b, c, d, e, f, g, h);
    //return forestFire(o, a, b, c, d, e, f, g, h);
    return forestFireWet(o, a, b, c, d, e, f, g, h);
    //return forestWet(o, a, b, c, d, e, f, g, h);
    //

    return call(o, a, b, c, d, e, f, g, h);
}

void CellularAutomaton::compile(QString s)
{
    if (engine) delete engine;
    if (function) delete function;
    source = s;
    engine = new QJSEngine();
    function = new QJSValue(engine->evaluate(source));
}

char CellularAutomaton::call(char o, char a, char b, char c, char d, char e, char f, char g, char h)
{
    //qWarning("callcallcallcall");
    QJSValue v(0);
    QJSValueList args;
    args << o << a << b << c << d << e << f << g << h;
    if (function->isCallable())
        v = function->call(args);
    return v.toInt();
}

void CellularAutomaton::setCounter(int c){ counter = c;}


//-- RENDERING --//

void CellularAutomaton::render(QImage &image)
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

bool CellularAutomaton::readFromJSON(QJsonObject &json)
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

bool CellularAutomaton::writeToJSON(QJsonObject &json)
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

//-- SOME RULES --//

const char life[9] = {0, 0, 1, 1, 0, 0, 0, 0, 0};
const char dead[9] = {0, 0, 0, 1, 0, 0, 0, 0, 0};

char conwayGameOfLife(char o, char a, char b, char c, char d, char e, char f, char g, char h)
{

    //if (o == 1) return life[a + b + c + d + e + f + g + h];
    //else return dead[a + b + c + d + e + f + g + h];


    int s = a + b + c + d + e + f + g + h;
    if (o == 1){
        if (s < 2) return 0;
        else if (s > 3) return 0;
        else return 1;
    }
    else if (s == 3) return 1;
    else return 0;

}

char rockPaperScissor(char o, char a, char b, char c, char d, char e, char f, char g, char h)
{
    int _r = 0; int _p = 1; int _s = 2;

    if (o == _r){ // rock
        int paper = 0;
        if (a == 0) paper++;
        if (b == 0) paper++;
        if (c == 0) paper++;
        if (d == 0) paper++;
        if (e == 0) paper++;
        if (f == 0) paper++;
        if (g == 0) paper++;
        if (h == 0) paper++;
        if (paper > 2) return _p;
        else return o;
    }
    else if (o == _p){ // paper
        int scissor = 0;
        if (a == 0) scissor++;
        if (b == 0) scissor++;
        if (c == 0) scissor++;
        if (d == 0) scissor++;
        if (e == 0) scissor++;
        if (f == 0) scissor++;
        if (g == 0) scissor++;
        if (h == 0) scissor++;
        if (scissor > 2) return _s;
        else return o;
    }
    else{ // scissor
        int rock = 0;
        if (a == 1) rock++;
        if (b == 1) rock++;
        if (c == 1) rock++;
        if (d == 1) rock++;
        if (e == 1) rock++;
        if (f == 1) rock++;
        if (g == 1) rock++;
        if (h == 1) rock++;
        if (rock > 2) return _r;
        else return o;
    }

}

char rockPaperScissorLizardSpock(char o, char a, char b, char c, char d, char e, char f, char g, char h)
{
    int _r = 0; int _p = 1; int _s = 2; int _l = 3; int _k = 4;

    if (o == _r){ // rock loses against paper and spock
        int _pC = 0;
        if (a == _p) _pC++;
        if (b == _p) _pC++;
        if (c == _p) _pC++;
        if (d == _p) _pC++;
        if (e == _p) _pC++;
        if (f == _p) _pC++;
        if (g == _p) _pC++;
        if (h == _p) _pC++;

        int _kC = 0;
        if (a == _k) _kC++;
        if (b == _k) _kC++;
        if (c == _k) _kC++;
        if (d == _k) _kC++;
        if (e == _k) _kC++;
        if (f == _k) _kC++;
        if (g == _k) _kC++;
        if (h == _k) _kC++;

        if (_pC > _kC){
            if (_pC > 2) return _p;
            else return o;
        }
        else {
            if (_kC > 2) return _k;
            else return o;
        }
    }
    else if (o == _p){ // paper loses against scissor and lizard
        int _sC = 0;
        if (a == _s) _sC++;
        if (b == _s) _sC++;
        if (c == _s) _sC++;
        if (d == _s) _sC++;
        if (e == _s) _sC++;
        if (f == _s) _sC++;
        if (g == _s) _sC++;
        if (h == _s) _sC++;

        int _lC = 0;
        if (a == _l) _lC++;
        if (b == _l) _lC++;
        if (c == _l) _lC++;
        if (d == _l) _lC++;
        if (e == _l) _lC++;
        if (f == _l) _lC++;
        if (g == _l) _lC++;
        if (h == _l) _lC++;

        if (_sC > _lC){
            if (_sC > 2) return _s;
            else return o;
        }
        else {
            if (_lC > 2) return _l;
            else return o;
        }
    }
    else if (o == _s){ // scissor loses against rock and spock
        int _rC = 0;
        if (a == _r) _rC++;
        if (b == _r) _rC++;
        if (c == _r) _rC++;
        if (d == _r) _rC++;
        if (e == _r) _rC++;
        if (f == _r) _rC++;
        if (g == _r) _rC++;
        if (h == _r) _rC++;

        int _kC = 0;
        if (a == _k) _kC++;
        if (b == _k) _kC++;
        if (c == _k) _kC++;
        if (d == _k) _kC++;
        if (e == _k) _kC++;
        if (f == _k) _kC++;
        if (g == _k) _kC++;
        if (h == _k) _kC++;

        if (_rC > _kC){
            if (_rC > 2) return _r;
            else return o;
        }
        else {
            if (_kC > 2) return _k;
            else return o;
        }
    }
    else if (o == _l){ // lizard loses against rock and scissor
        int _rC = 0;
        if (a == _r) _rC++;
        if (b == _r) _rC++;
        if (c == _r) _rC++;
        if (d == _r) _rC++;
        if (e == _r) _rC++;
        if (f == _r) _rC++;
        if (g == _r) _rC++;
        if (h == _r) _rC++;

        int _sC = 0;
        if (a == _s) _sC++;
        if (b == _s) _sC++;
        if (c == _s) _sC++;
        if (d == _s) _sC++;
        if (e == _s) _sC++;
        if (f == _s) _sC++;
        if (g == _s) _sC++;
        if (h == _s) _sC++;

        if (_rC > _sC){
            if (_rC > 2) return _r;
            else return o;
        }
        else {
            if (_sC > 2) return _s;
            else return o;
        }
    }
    else if (o == _k){ // spock loses against paper and lizard
        int _pC = 0;
        if (a == _p) _pC++;
        if (b == _p) _pC++;
        if (c == _p) _pC++;
        if (d == _p) _pC++;
        if (e == _p) _pC++;
        if (f == _p) _pC++;
        if (g == _p) _pC++;
        if (h == _p) _pC++;

        int _lC = 0;
        if (a == _l) _lC++;
        if (b == _l) _lC++;
        if (c == _l) _lC++;
        if (d == _l) _lC++;
        if (e == _l) _lC++;
        if (f == _l) _lC++;
        if (g == _l) _lC++;
        if (h == _l) _lC++;

        if (_pC > _lC){
            if (_pC > 2) return _p;
            else return o;
        }
        else {
            if (_lC > 2) return _l;
            else return o;
        }
    }


    else return o;
}

char forestFire(char o, char a, char b, char c, char d, char e, char f, char g, char h)
{
    int _b = 0; // burned
    int _f = 1; // fire
    int _t = 2; // tree or obstacle
    int _g = 3; // grass
    if (o == _b) return _b; // burned maintains burned
    else if (o == _f) {
        if ((a == _g) || (b == _g) || (c == _g) || (d == _g) ||
            (e == _g) || (f == _g) || (g == _g) || (h == _g))
            return _f;
        else if ((a == _t) || (b == _t) || (c == _t) || (d == _t) ||
                 (e == _t) || (f == _t) || (g == _t) || (h == _t))
            return _f;
        else return _b;
    }
    else if (o == _t){

        if ((a == _f) || (b == _f) || (c == _f) || (d == _f) ||
            (e == _f) || (f == _f) || (g == _f) || (h == _f)){
            int x = (std::rand() % 2);
            if (x == 0) return _g;
            else return _t;
        }
        else return _t;
    }
    else if (o == _g){ //
        if ((a == _f) || (c == _f) || (e == _f) || (g == _f)) // direct
            return _f;
        else if ((b == _f) || (d == _f) || (f == _f) || (h == _f)){
            // diagonal (needs probability)
            int x = (std::rand()%100);
            if (x < 41) return _f;
            else return _g;
        }
        else return _g;
    }
    return o;
}


char forestFireWet(char o, char a, char b, char c, char d, char e, char f, char g, char h)
{
    int _b = 0; // burned
    int _f = 1; // fire
    int _t = 2; // tree or obstacle
    int _g = 3; // wet grass
    int _d = 4; // dry grass
    if (o == _b) return _b; // burned maintains burned
    else if (o == _f) {
        if ((a == _g) || (b == _g) || (c == _g) || (d == _g) ||
            (e == _g) || (f == _g) || (g == _g) || (h == _g))
            return _f;
        else if ((a == _t) || (b == _t) || (c == _t) || (d == _t) ||
                 (e == _t) || (f == _t) || (g == _t) || (h == _t))
            return _f;
        else return _b;
    }
    else if (o == _t){

        if ((a == _f) || (b == _f) || (c == _f) || (d == _f) ||
            (e == _f) || (f == _f) || (g == _f) || (h == _f)){
            int x = (std::rand() % 2);
            if (x == 0) return _g;
            else return _t;
        }
        else return _t;
    }
    else if (o == _g){
        if ((a == _f) || (c == _f) || (e == _f) || (g == _f)) // direct
            return _d;
        else if ((b == _f) || (d == _f) || (f == _f) || (h == _f)){
            // diagonal (needs probability)
            int x = (std::rand()%100);
            if (x < 41) return _d;
            else return _g;
        }
        else return _g;
    }
    else if (o == _d){ //
        if ((a == _f) || (c == _f) || (e == _f) || (g == _f)) // direct
            return _f;
        else if ((b == _f) || (d == _f) || (f == _f) || (h == _f)){
            // diagonal (needs probability)
            int x = (std::rand()%100);
            if (x < 41) return _f;
            else return _d;
        }
        else return _d;
    }
    return o;
}

char forestWet(char o, char a, char b, char c, char d, char e, char f, char g, char h)
{
    int _b = 0; // burned
    int _f = 1; // fire
    int _t = 2; // tree or obstacle
    int _g = 3; // wet grass
    int _d = 4; // dry grass
    if (o == _d){
        if ((a == _g) || (c == _g) || (e == _g) || (g == _g)) // direct
            return _g;
        else if ((b == _g) || (d == _g) || (f == _g) || (h == _g)){
            // diagonal (needs probability)
            int x = (std::rand()%100);
            if (x < 41) return _g;
            else return _d;
        }
        else return _d;
    }
    else return o;
}
