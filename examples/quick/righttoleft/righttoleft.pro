TEMPLATE = app

QT += quick qml
SOURCES += main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/qtquick/righttoleft
qml.files = righttoleft.qml layoutdirection layoutmirroring textalignment
qml.path = $$[QT_INSTALL_EXAMPLES]/qtquick/righttoleft
sources.files = $$SOURCES righttoleft.pro
sources.path = $$qml.path
INSTALLS += sources target qml
