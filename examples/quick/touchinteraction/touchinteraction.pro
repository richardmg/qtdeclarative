TEMPLATE = app

QT += quick qml
SOURCES += main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/qtquick/touchinteraction
qml.files = flickable multipointtouch pincharea touchinteraction.qml
qml.path = $$[QT_INSTALL_EXAMPLES]/qtquick/touchinteraction
sources.files = $$SOURCES touchinteraction.pro
sources.path = $$qml.path
INSTALLS += sources target qml
