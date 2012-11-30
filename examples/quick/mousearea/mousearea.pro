TEMPLATE = app

QT += quick qml
SOURCES += main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/qtquick/mousearea
qml.files = mousearea.qml
qml.path = $$[QT_INSTALL_EXAMPLES]/qtquick/mousearea
sources.files = $$SOURCES mousearea.pro
sources.path = $$qml.path
INSTALLS += sources target qml
