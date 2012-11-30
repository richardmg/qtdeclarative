TEMPLATE = app

QT += quick qml
SOURCES += main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/qtqml/xmlhttprequest
qml.files = xmlhttprequest.qml
qml.path = $$[QT_INSTALL_EXAMPLES]/qtqml/xmlhttprequest
INSTALLS += target qml
