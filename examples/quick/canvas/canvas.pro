TEMPLATE = app

QT += quick qml
SOURCES += main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/qtquick/canvas
qml.files = canvas.qml bezierCurve clip quadraticCurveTo roundedrect smile squircle tiger contents
qml.path = $$[QT_INSTALL_EXAMPLES]/qtquick/canvas
sources.files = $$SOURCES canvas.pro
sources.path = $$qml.path
INSTALLS += sources target qml
