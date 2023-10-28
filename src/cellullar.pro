QT       += core gui widgets qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    about.cpp \
    cellularautomaton.cpp \
    cellularwidget.cpp \
    config.cpp \
    editdlg.cpp \
    jshilite.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    about.h \
    builddatetime.h \
    cellularautomaton.h \
    cellularwidget.h \
    config.h \
    editdlg.h \
    jshilite.h \
    mainwindow.h

FORMS += \
    about.ui \
    config.ui \
    editdlg.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    cellullar.qrc

RC_ICONS = cellullar.ico
