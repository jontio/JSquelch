QT       += core gui
TARGET = jsquelch

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CI {
  SOURCES += \
    tests/testall.cpp \
    tests/dsp_movingaverage_tests.cpp \
    tests/dsp_vectormovingaverage_tests.cpp \
    tests/dsp_vectormovingvariance_tests.cpp \
    tests/dsp_vectormovingmax_tests.cpp \
    tests/dsp_vectormovingmin_tests.cpp \
    tests/dsp_hann_tests.cpp \
    tests/dsp_overlappedrealfft_tests.cpp \
    tests/dsp_vectormovingminwithassociate_tests.cpp \
    tests/dsp_movingnoiseestimator_tests.cpp
  LIBS += -lCppUTest

} else {
  SOURCES += \
        src/main.cpp\
}

SOURCES += \
    ../JFFT/jfft.cpp \
    src/jsquelch.cpp \
    src/qled/qled.cpp \
    src/dsp/dsp.cpp \
    src/util/RuntimeError.cpp


HEADERS += \
    ../JFFT/jfft.h \
    src/jsquelch.h \
    src/qled/qled.h \
    src/dsp/dsp.h \
    src/util/RuntimeError.h

FORMS += \
    src/jsquelch.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc



