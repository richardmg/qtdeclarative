TEMPLATE = app

QT += quick qml
SOURCES += main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/qtquick/draganddrop
qml.files = draganddrop.qml tiles views
qml.path = $$[QT_INSTALL_EXAMPLES]/qtquick/draganddrop
sources.files = $$SOURCES draganddrop.pro
sources.path = $$qml.path
INSTALLS += sources target qml
