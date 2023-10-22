#ifndef JSHILITE_H
#define JSHILITE_H

#include <QtWidgets>

#define CR '\x0D'
#define LF '\x0A'

#define JS_NOTHING              0
#define JS_LINE_COMMENT         1
#define JS_MULTILINE_COMMENT    2
#define JS_IDENTIFIER           3
#define JS_RESERVED             4
#define JS_RESERVED_2           5
#define JS_DECIMAL_NUMBER       6
#define JS_HEX_NUMBER           7
#define JS_OCTAL_NUMBER         8
#define JS_BINARY_NUMBER        9
#define JS_OPERATOR             10
#define JS_DELIMITER            11
#define JS_STRING_DOUBLE        12
#define JS_STRING_SINGLE        13
#define JS_TEMPLATE             14
#define JS_REGEX                15
#define JS_EOL                  16
#define JS_SPACES               17
#define JS_UNKNOWN              18
#define JS_RESERVED_RETURN_CASE 19
#define JS_DELIMITER_CLOSE_PB   20

class JSHighlighter: public QSyntaxHighlighter
{
    Q_OBJECT

public:
    JSHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;
    void highlight(int i, int t, int tk);
public:
    QTextCharFormat formats[19];
};

#endif // JSHILITE_H
