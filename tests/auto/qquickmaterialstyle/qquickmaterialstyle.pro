TEMPLATE = app
TARGET = tst_qquickmaterialstyle
CONFIG += qmltestcase

SOURCES += \
    $$PWD/tst_qquickmaterialstyle.cpp

RESOURCES += \
    $$PWD/qtquickcontrols2.conf

OTHER_FILES += \
    $$PWD/data/*

TESTDATA += \
    $$PWD/data/tst_*
