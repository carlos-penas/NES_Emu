QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        Exceptions/notmappedaddressexception.cpp \
        Mappers/mapper.cpp \
        Mappers/mapper0.cpp \
        Mappers/mapper2.cpp \
        bus.cpp \
        cartridge.cpp \
        cpu.cpp \
        cpuinstruction.cpp \
        interrupt.cpp \
        main.cpp \
        nes.cpp \
        ppu.cpp \
        utils.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Exceptions/notmappedaddressexception.h \
    Mappers/mapper.h \
    Mappers/mapper0.h \
    Mappers/mapper2.h \
    bus.h \
    cartridge.h \
    compilationSettings.h \
    constants.h \
    cpu.h \
    cpuOpCodes.h \
    cpuinstruction.h \
    interrupt.h \
    nes.h \
    ppu.h \
    types.h \
    utils.h

#-----SFML graphics library-----
INCLUDEPATH += $$PWD/../../../../usr/include/SFML
DEPENDPATH += $$PWD/../../../../usr/include/SFML

unix:!macx: LIBS += -L$$PWD/../../../../usr/lib/x86_64-linux-gnu/ -lsfml-graphics
unix:!macx: LIBS += -L$$PWD/../../../../usr/lib/x86_64-linux-gnu/ -lsfml-window
unix:!macx: LIBS += -L$$PWD/../../../../usr/lib/x86_64-linux-gnu/ -lsfml-system
#-------------------------------
