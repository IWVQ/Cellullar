#include "jshilite.h"

JSHighlighter::JSHighlighter(QTextDocument *parent):
    QSyntaxHighlighter(parent)
{
    QTextCharFormat f;

    formats[JS_NOTHING          ] = f;

    f.setForeground(Qt::darkGreen);
    formats[JS_LINE_COMMENT     ] = f;
    formats[JS_MULTILINE_COMMENT] = f;

    f.setForeground(Qt::black);
    formats[JS_IDENTIFIER       ] = f;

    f.setForeground(Qt::blue);
    formats[JS_RESERVED         ] = f;

    f.setForeground(Qt::darkMagenta);
    formats[JS_RESERVED_2       ] = f;

    f.setForeground(QColor("#ff8000"));
    formats[JS_DECIMAL_NUMBER   ] = f;
    formats[JS_HEX_NUMBER       ] = f;
    formats[JS_OCTAL_NUMBER     ] = f;
    formats[JS_BINARY_NUMBER    ] = f;

    f.setForeground(Qt::darkBlue);
    formats[JS_OPERATOR         ] = f;
    formats[JS_DELIMITER        ] = f;

    f.setForeground(QColor("#ff0080"));
    formats[JS_STRING_DOUBLE    ] = f;
    formats[JS_STRING_SINGLE    ] = f;
    formats[JS_TEMPLATE         ] = f;

    f.setForeground(Qt::darkRed);
    formats[JS_REGEX            ] = f;

    f.setForeground(Qt::black);
    formats[JS_EOL              ] = f;
    formats[JS_SPACES           ] = f;

    f.setForeground(Qt::red);
    formats[JS_UNKNOWN          ] = f;
}

void JSHighlighter::highlight(int i, int t, int tk)
{
    if (JS_RESERVED_RETURN_CASE == tk)
        tk = JS_RESERVED;
    if (JS_DELIMITER_CLOSE_PB == tk)
        tk = JS_DELIMITER;
    if (t != i)
        setFormat(i, t - i, formats[tk]);
}

bool isEoL(QChar c)
{
    return (c == 0x10) || (c == 0x13);
}

bool isNumberChar(QChar c)
{
    return ((c >= '0') && (c <= '9'));
}

bool isHexChar(QChar c)
{
    return isNumberChar(c) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'));
}

bool isOctalChar(QChar c)
{
    return ((c >= '0') && (c <= '7'));
}

bool isBinaryChar(QChar c)
{
    return (c == '0') || (c == '1');
}

bool isWordChar(QChar c)
{
    return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))
            || (c == '_') || (c == '$');
}

bool isReservedReturnCase(const QString &s)
{
    if ((s == "return") ||
        (s == "case"))
        return true;
    return false;
}

bool isReservedWord(const QString &s)
{
    if ((s == "abstract") ||
        (s == "arguments") ||
        (s == "await") ||
        (s == "boolean") ||
        (s == "break") ||
        (s == "byte") ||
        (s == "case") ||
        (s == "catch") ||
        (s == "char") ||
        (s == "class") ||
        (s == "const") ||
        (s == "continue") ||
        (s == "debugger") ||
        (s == "default") ||
        (s == "delete") ||
        (s == "do") ||
        (s == "double") ||
        (s == "else") ||
        (s == "enum") ||
        (s == "eval") ||
        (s == "export") ||
        (s == "extends") ||
        (s == "false") ||
        (s == "final") ||
        (s == "finally") ||
        (s == "float") ||
        (s == "for") ||
        (s == "function") ||
        (s == "goto") ||
        (s == "if") ||
        (s == "implements") ||
        (s == "import") ||
        (s == "in") ||
        (s == "instanceof") ||
        (s == "int") ||
        (s == "interface") ||
        (s == "let") ||
        (s == "long") ||
        (s == "native") ||
        (s == "new") ||
        (s == "null") ||
        (s == "package") ||
        (s == "private") ||
        (s == "protected") ||
        (s == "public") ||
        (s == "return") ||
        (s == "short") ||
        (s == "static") ||
        (s == "super") ||
        (s == "switch") ||
        (s == "synchronized") ||
        (s == "this") ||
        (s == "throw") ||
        (s == "throws") ||
        (s == "transient") ||
        (s == "true") ||
        (s == "try") ||
        (s == "typeof") ||
        (s == "var") ||
        (s == "void") ||
        (s == "volatile") ||
        (s == "while") ||
        (s == "with") ||
        (s == "yeld"))
        return true;
    return false;
}

bool isReservedWord2(const QString &s)
{
    if ((s == "Infinity") ||
        (s == "NaN") ||
        (s == "undefined") ||
        (s == "Math") ||
        (s == "Array") ||
        (s == "Date") ||
        (s == "String") ||
        (s == "Number") ||
        (s == "Object"))
        return true;
    return false;
}

bool isOperatorChar(QChar c)
{
    return  (c == '|') || (c == '!') || (c == '#') || (c == '%') ||
            (c == '&') || (c == '/') || (c == '=') || (c == '\\') ||
            (c == '?') || (c == '+') || (c == '*') || (c == '~') ||
            (c == '^') || (c == '-') || (c == '.') || (c == ':') ||
            (c == '<') || (c == '>') || (c == '@');
}

bool isDelimiterChar(QChar c)
{
    return  (c == '(') || (c == ')') || (c == '[') || (c == ']') ||
            (c == '{') || (c == '}') || (c == ',') || (c == ';');
}

bool isSpaceChar(QChar c)
{
    return (c == ' ') || (c == '\x09');
}

bool isPrevTokenForRegex(int token)
{
    return !((token == JS_IDENTIFIER) || (token == JS_DECIMAL_NUMBER) ||
            (token == JS_HEX_NUMBER) || (token == JS_OCTAL_NUMBER) ||
            (token == JS_BINARY_NUMBER) || (token == JS_RESERVED) ||
            (token == JS_RESERVED_2) || (token == JS_DELIMITER_CLOSE_PB));
}

bool consumeEoL(const QString &text, int &i, int l)
{
    if ((i < l) && (text[i] == CR)){
        i++;
        if ((i < l) && (text[i] == LF))
            i++;
        return true;
    }
    else if ((i < l) && (text[i] == LF)){
        i++;
        return true;
    }
    return false;
}

void JSHighlighter::highlightBlock(const QString &text)
{
    //QByteArray text;
    //str.append(text);
    int l = text.length();
    int i = 0;
    int token = JS_NOTHING;
    int f = 0;

    if (previousBlockState() == JS_MULTILINE_COMMENT){
        f = i;
        goto LBL_CONTINUE_COMMENT;
    }
    else if (previousBlockState() == JS_STRING_DOUBLE){
        f = i;
        goto LBL_CONTINUE_STRING_DOUBLE;
    }
    else if (previousBlockState() == JS_STRING_SINGLE){
        f = i;
        goto LBL_CONTINUE_STRING_SINGLE;
    }
    else if (previousBlockState() == JS_TEMPLATE){
        f = i;
        goto LBL_CONTINUE_TEMPLATE;
    }

    setCurrentBlockState(JS_NOTHING);
LBL_AGAIN:

    if (i >= l) goto LBL_END;

    // posible comment
    if (text[i] == '/'){
        if (i + 1 < l){
            if (text[i + 1] == '/'){
                // line comment
                f = i;
                i += 2;
                while ((i < l) && (!isEoL(text[i]))) i++;
                highlight(f, i, JS_LINE_COMMENT);
            }
            else if (text[i + 1] == '*'){
                // multiline comment
                f = i;
                i += 2;
LBL_CONTINUE_COMMENT:
                bool commentclosed = false;
                while (i < l){
                    if (text[i] == '*'){
                        i++;
                        if ((i < l) && (text[i] == '/')){
                            i++;
                            commentclosed = true;
                            break;
                        }
                    }
                    i++;
                }
                highlight(f, i, JS_MULTILINE_COMMENT);
                if (!commentclosed)
                    setCurrentBlockState(JS_MULTILINE_COMMENT);
            }
        }
    }

    if (i >= l) goto LBL_END;

    // numbers
    if (isNumberChar(text[i])){
        // decimal, hex, octal and binary
        f = i;
        token = JS_DECIMAL_NUMBER;
        QChar x = text[i];
        i++;
        if (i < l){
            if ((x == '0') && ((text[i] == 'x') || (text[i] == 'X'))){
                // hexadecimal
                i++;
                while ((i < l) && isHexChar(text[i])) i++;
                token = JS_HEX_NUMBER;
            }
            else if ((x == '0') && ((text[i] == 'o') || (text[i] == 'O'))){
                // octal
                i++;
                while ((i < l) && isOctalChar(text[i])) i++;
                token = JS_OCTAL_NUMBER;
            }
            else if ((x == '0') && ((text[i] == 'b') || (text[i] == 'B'))){
                // binary
                i++;
                while ((i < l) && isBinaryChar(text[i])) i++;
                token = JS_BINARY_NUMBER;
            }
            else {
                // decimal
                while ((i < l) && isNumberChar(text[i])) i++;
                if ((i < l) && (text[i] == '.')){
                    i++;
                    while ((i < l) && isNumberChar(text[i])) i++;
                    if ((i < l) && ((text[i] == 'e') || (text[i] == 'E'))){
                        i++;
                        if ((i < l) && ((text[i] == '+') || (text[i] == '-')))
                            i++;
                        while ((i < l) && isNumberChar(text[i])) i++;
                    }
                }
                token = JS_DECIMAL_NUMBER;
            }
        }
        highlight(f, i, token);
    }
    // words
    else if (isWordChar(text[i])){
        // identifiers and reserveds
        f = i;
        while ((i < l) && isWordChar(text[i])) i++;
        //QByteArray d = str.mid(f, i - f);
        QString s = text.mid(f, i - f);
        int tk = 0;
        if (isReservedReturnCase(s)) // special case for regex
            token = JS_RESERVED_RETURN_CASE;
        else if (isReservedWord(s))
            token = JS_RESERVED;
        else if (isReservedWord2(s))
            token = JS_RESERVED_2 | tk;
        else
            token = JS_IDENTIFIER;
        highlight(f, i, token);
    }
    // strings
    else if (text[i] == '"'){
        // double quotation string
        f = i;
        i++;
LBL_CONTINUE_STRING_DOUBLE:
        token = JS_STRING_DOUBLE;
        while ((i < l) && (text[i] != '"') && !isEoL(text[i])){
            if (text[i] == '\\'){
                i++;
                consumeEoL(text, i, l);
                if (i == l) setCurrentBlockState(token);
            }
            i++;
        }
        if ((i < l) && (text[i] == '"')) i++;
        highlight(f, i, token);
    }
    else if (text[i] == '\''){
        // single quotation string
        f = i;
        i++;
LBL_CONTINUE_STRING_SINGLE:
        token = JS_STRING_SINGLE;
        while ((i < l) && (text[i] != '\'') && !isEoL(text[i])){
            if (text[i] == '\\'){
                i++;
                consumeEoL(text, i, l);
                if (i == l) setCurrentBlockState(token);
            }
            i++;
        }
        if ((i < l) && (text[i] == '\'')) i++;
        highlight(f, i, token);
    }
    else if (text[i] == '`'){
        // template
        f = i;
        i++;
LBL_CONTINUE_TEMPLATE:
        token = JS_TEMPLATE;
        while ((i < l) && (text[i] != '`') && !isEoL(text[i])){
            if (text[i] == '\\'){
                i++;
                consumeEoL(text, i, l);
                if (i == l) setCurrentBlockState(token);
            }
            i++;
        }
        if ((i < l) && (text[i] == '`')) i++;
        highlight(f, i, token);
    }
    else if ((text[i] == '/') && isPrevTokenForRegex(token)){
        f = i;
        i++;
        token = JS_REGEX;
        while ((i < l) && (text[i] != '/') && !isEoL(text[i])){
            if (text[i] == '\\') i++;
            i++;
        }
        if ((i < l) && (text[i] == '/')){
            i++;
            if ((i < l) && ((text[i] == 'i') || (text[i] == 'g') || (text[i] == 'm')))
               i++;
        }
        highlight(f, i, token);
    }
    // operators
    else if (isOperatorChar(text[i])){
        f = i;
        token = JS_OPERATOR;
        while ((i < l) && isOperatorChar(text[i])) i++;
        highlight(f, i, token);
    }
    else if (isDelimiterChar(text[i])){
        f = i;
        if ((text[i] == ')') || (text[i] == ']'))
            token = JS_DELIMITER_CLOSE_PB;
        else
            token = JS_DELIMITER;
        i++;
        highlight(f, i, token);
    }
    // newline
    else if (isEoL(text[i])){
        f = i;
        token = JS_EOL;
        consumeEoL(text, i, l);
        highlight(f, i, token);
    }
    // spaces
    else if (isSpaceChar(text[i])){
        f = i;
        while ((i < l) && isSpaceChar(text[i])) i++;
        highlight(f, i, JS_SPACES); // don't store in token
    }
    // unknown
    else{
        token = JS_UNKNOWN;
        highlight(i, i + 1, token);
        i++;
    }

    goto LBL_AGAIN;

LBL_END:

    return;
}
