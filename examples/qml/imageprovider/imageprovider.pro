TEMPLATE = lib
CONFIG += qt plugin
QT += qml quick

DESTDIR = ImageProviderCore
TARGET  = qmlimageproviderplugin

SOURCES += imageprovider.cpp

OTHER_FILES += imageprovider.json

sources.files = $$SOURCES imageprovider.qml imageprovider.pro $$OTHER_FILES
sources.path = $$[QT_INSTALL_EXAMPLES]/qtqml/imageprovider

target.path = $$[QT_INSTALL_EXAMPLES]/qtqml/imageprovider/ImageProviderCore

ImageProviderCore_sources.files = \
    ImageProviderCore/qmldir 
ImageProviderCore_sources.path = $$[QT_INSTALL_EXAMPLES]/qtqml/imageprovider/ImageProviderCore

INSTALLS = sources ImageProviderCore_sources target
