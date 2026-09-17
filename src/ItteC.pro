QT += widgets

CONFIG += c++17

SOURCES += \
    CanvasModel.cpp \
    CanvasView.cpp \
    ColorModel.cpp \
    ColorPickerDialog.cpp \
    ColorWheel.cpp \
    EditorController.cpp \
    GradientSlider.cpp \
    MainWindow.cpp \
    main.cpp

HEADERS += \
    CanvasModel.h \
    CanvasView.h \
    ColorModel.h \
    ColorPickerDialog.h \
    ColorWheel.h \
    EditorController.h \
    GradientSlider.h \
    MainWindow.h

FORMS += \
    MainWindow.ui

win32: RC_FILE = ItteC.rc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
