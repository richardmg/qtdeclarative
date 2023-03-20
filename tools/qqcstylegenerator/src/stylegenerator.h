// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QSTYLEREADER_H
#define QSTYLEREADER_H

#include <QtCore>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDir>

#include "jsontools.h"

using namespace JsonTools;

struct Strectch9p {
    const QJsonArray horizontal;
    const QJsonArray vertical;
};

class StyleGenerator {

public:
    StyleGenerator(const QString &fileId, const QString &token
        , const QString &targetPath, bool verbose, bool silent)
        : m_fileId(fileId)
        , m_token(token)
        , m_targetPath(targetPath)
        , m_verbose(verbose)
        , m_silent(silent)
    {
    }

    void generateStyle()
    {
        debug("Generate style: " + m_targetPath);

        downloadFigmaDocument();
        generateControls();
        downloadImages();
        debugHeader("Generating config files");
        generateQmlDir();
        generateConfiguration();
    }

private:

    /*
    void generateButton()
    {
        generateControl("Button");

        const QMap<QString, QString> stateMap = {
            {"idle", ""},
            {"blocked", "disabled"}
        };

        forEachStateIn("ButtonTemplate",
            [this, &stateMap](const QString &stateName, const QJsonObject &stateComponent){

            const auto imageState = stateMap.value(stateName, stateName);
            const auto bg = findNamedChild({"background"}, stateComponent);
            const auto label = findNamedChild({"label"}, stateComponent);
            auto config = createConfiguration(bg, label);

            generateImage("button-background", imageState, bg);
            addConfiguration("button-background", imageState, config);
        });
    }

    void generateCheckBox()
    {
        generateControl("CheckBox");

        const QMap<QString, QString> stateMap = {
            {"idle", ""},
            {"blocked", "disabled"},
            {"checkedBlocked", "disabled-checked"},
            {"checkedPressed", "checked-pressed"},
            {"checkedHovered", "checked-hovered"},
            {"checkedTri", "tristate"},
            {"blockedTri", "tristate-disabled"},
            {"pressedTri", "tristate-pressed"},
            {"hoveredTri", "tristate-hovered"}
        };

        forEachStateIn("CheckboxTemplate",
            [this, &stateMap](const QString &stateName, const QJsonObject &stateComponent){

            const auto imageState = stateMap.value(stateName, stateName);
            const auto bg = findNamedChild({"CheckboxBackground", "background"}, stateComponent);
            const auto label = findNamedChild({"CheckboxBackground", "label"}, stateComponent);
            const auto indicatorBg = findNamedChild({"CheckboxIndicator", "checkBackground"}, stateComponent);
            const auto indicatorIcon = findNamedChild({"CheckboxIndicator", "checkIcon"}, stateComponent);
            const auto indicatorTriIcon = findNamedChild({"CheckboxIndicator", "triCheckIcon"}, stateComponent);

            generateImage("checkbox-background", imageState, bg);
            generateImage("checkbox-indicator-background", imageState, indicatorBg);
            generateImage("checkbox-indicator-icon", imageState, indicatorIcon);
            generateImage("checkbox-indicator-partialicon", imageState, indicatorTriIcon);

            auto config = createConfiguration(bg, label);
            const QRectF bgGeo = getGeometry(bg);
            const QRectF labelGeo = getGeometry(label);
            const QRectF indicatorGeo = getGeometry(indicatorBg);
            const bool mirrored = indicatorGeo.x() > bgGeo.width() / 2;
            config.insert("mirrored", mirrored);

            if (mirrored) {
                // When we detect that the indicator is on the right side of
                // the background, we create a control that is mirrored.
                config.insert("leftPadding", labelGeo.x());
                config.insert("rightPadding", qMax(0., bgGeo.width() - indicatorGeo.x() - indicatorGeo.width()));
                config.insert("spacing", qMax(0., indicatorGeo.x() - labelGeo.x() - labelGeo.width()));
            } else {
                config.insert("leftPadding", indicatorGeo.x());
                config.insert("rightPadding",  qMax(0., bgGeo.width() - labelGeo.x() - labelGeo.width()));
                config.insert("spacing", qMax(0., labelGeo.x() - indicatorGeo.x() - indicatorGeo.width()));
            }

            addConfiguration("checkbox-background", imageState, config);
        });
    }

    void generateSwitch()
    {
        generateControl("Switch");

        const QMap<QString, QString> stateMap = {
            {"idleON", "checked"},
            {"idleOFF", ""},
            {"pressedON", "checked-pressed"},
            {"pressedOFF", "pressed"},
            {"blocked", "disabled"}
        };

        forEachStateIn("SwitchTemplate",
            [this, &stateMap](const QString &stateName, const QJsonObject &stateComponent){

            const QString imageState = stateMap.value(stateName, stateName);
            const auto bg = findNamedChild({"SwitchBackground", "background"}, stateComponent);
            const auto bgIconLeft = findNamedChild({"SwitchBackground", "iconLeftOff"}, stateComponent);
            const auto bgIconRight = findNamedChild({"SwitchBackground", "iconRightOff"}, stateComponent);
            const auto handle = findNamedChild({"SwitchHandle", "handle"}, stateComponent);
            const auto handleIconLeft = findNamedChild({"SwitchHandle", "iconLeftON"}, stateComponent);
            const auto handleIconRight = findNamedChild({"SwitchHandle", "iconRightON"}, stateComponent);

            generateImage("switch-background", imageState, bg);
            generateImage("switch-background-lefticon", imageState, bgIconLeft);
            generateImage("switch-background-righticon", imageState, bgIconRight);
            generateImage("switch-handle", imageState, handle);
            generateImage("switch-handle-lefticon", imageState, handleIconLeft);
            generateImage("switch-handle-righticon", imageState, handleIconRight);

            // The figma template should have a 'label' so we know where to place the contents.
            // But it doesn't, so hard-code a contentGeo for now!
            // const QRectF contentGeo = getGeometry(label);
            auto config = createConfiguration(bg, bg); // TODO: second arg should be the label!
            const QRectF bgGeo = getGeometry(bg);
            const QRectF labelGeo(bgGeo.x() + bgGeo.width() + 6, 0, 50, bgGeo.height());
            const qreal spacing = labelGeo.x() - bgGeo.x() - bgGeo.width();
            config.insert("spacing", spacing);
            config.insert("leftPadding", bgGeo.x() + bgGeo.width() + spacing);
            config.insert("topPadding", labelGeo.y());
            config.insert("rightPadding", qMax(0., bgGeo.width() - labelGeo.x() - labelGeo.width()));
            config.insert("bottomPadding", qMax(0., bgGeo.height() - labelGeo.y() - labelGeo.height()));
            addConfiguration("switch-background", imageState, config);
        });
    }
    */

    void downloadFigmaDocument()
    {
        debug("downloading figma file");
        newProgress("downloading figma file");
        QJsonDocument figmaResponsDoc;
        const QUrl url("https://api.figma.com/v1/files/" + m_fileId);
        debug("requesting: " + url.toString());

        QNetworkRequest request(url);
        request.setRawHeader(QByteArray("X-FIGMA-TOKEN"), m_token.toUtf8());
        debug("using token: " + m_token.toUtf8());

        QScopedPointer<QNetworkAccessManager> manager(new QNetworkAccessManager);
        manager->setAutoDeleteReplies(true);

        QNetworkReply *reply = manager->get(request);

        QObject::connect(reply, &QNetworkReply::finished, [this, &reply]{
            if (reply->error() == QNetworkReply::NoError)
                m_document = QJsonDocument::fromJson(reply->readAll());

            if (qgetenv("QSTYLEGENERATOR_SAVEDOC") == "true")
                saveForDebug(m_document.object(), "figmastyle.json");
        });

        QObject::connect(reply, &QNetworkReply::downloadProgress, [this]{
            progress();
        });

        while (reply->isRunning())
            qApp->processEvents();

        if (reply->error() != QNetworkReply::NoError)
            throw RestCallException(QStringLiteral("Could not download design file from Figma: ")
                + networkErrorString(reply));
    }

    QJsonDocument generateImageUrls()
    {
        newProgress("downloading image urls");
        QJsonDocument figmaResponsDoc;
        const QString ids = QStringList(m_imagesToDownload.keys()).join(",");
        const QUrl url("https://api.figma.com/v1/images/" + m_fileId + "?ids=" + ids);
        debug("request: " + url.toString());
        QNetworkRequest request(url);
        request.setRawHeader(QByteArray("X-FIGMA-TOKEN"), m_token.toUtf8());

        QScopedPointer<QNetworkAccessManager> manager(new QNetworkAccessManager);
        manager->setAutoDeleteReplies(true);

        QNetworkReply *reply = manager->get(request);

        QObject::connect(reply, &QNetworkReply::finished, [reply, &figmaResponsDoc]{
            if (reply->error() == QNetworkReply::NoError)
                figmaResponsDoc = QJsonDocument::fromJson(reply->readAll());
        });

        QObject::connect(reply, &QNetworkReply::downloadProgress, [this]{
            progress();
        });

        while (reply->isRunning())
            qApp->processEvents();

        if (reply->error() != QNetworkReply::NoError)
            throw RestCallException(QStringLiteral("Could not download images from Figma: ")
                + networkErrorString(reply));

        return figmaResponsDoc;
    }

    void downloadImages(const QJsonDocument &figmaImagesResponsDoc)
    {
        debug("downloading requested images...");
        newProgress("downloading images");
        const auto imageUrls = getObject("images", figmaImagesResponsDoc.object());

        // Create image directory
        const QString imageDir = m_targetPath + "/images/";
        QFileInfo fileInfo(imageDir);
        const QDir targetDir = fileInfo.absoluteDir();
        if (!targetDir.exists()) {
            if (!QDir().mkpath(targetDir.path()))
                throw std::runtime_error("Could not create image directory: " + imageDir.toStdString());
        }

        QScopedPointer<QNetworkAccessManager> manager(new QNetworkAccessManager);
        manager->setAutoDeleteReplies(true);
        int requestCount = imageUrls.keys().count();

        for (const QString &key : imageUrls.keys()) {
            const QString imageUrl = imageUrls.value(key).toString();
            if (imageUrl.isEmpty()) {
                qWarning().noquote().nospace() << "No image URL generated for id: " << key << ", " << m_imagesToDownload.value(key);
                requestCount--;
                continue;
            }
            QNetworkReply *reply = manager->get(QNetworkRequest(imageUrl));

            QObject::connect(reply, &QNetworkReply::finished, [this, key, imageUrl, imageDir, reply, &requestCount] {
                requestCount--;
                if (reply->error() != QNetworkReply::NoError) {
                    qWarning().nospace().noquote() << "Failed to download "
                        << imageUrl << " (id: " << key << "). "
                        << "Error code:" << networkErrorString(reply);
                    return;
                }

                const QString name = m_imagesToDownload.value(key);
                const QString path = imageDir + name + ".png";

                QPixmap pixmap;
                pixmap.loadFromData(reply->readAll(), "png");
                pixmap.save(path);
                debug("downloaded image: " + imageUrl + " (id: " + key + ")" + " to " + path);
                progress();
            });
        }

        while (requestCount > 0)
            qApp->processEvents();
    }

    void downloadImages()
    {
        debugHeader("Downloading images from Figma");
        if (m_imagesToDownload.isEmpty()) {
            debug("No images to download!");
            return;
        }

        try {
            downloadImages(generateImageUrls());
        } catch (std::exception &e) {
            qWarning().nospace().noquote() << "Could not generate images: " << e.what();
        }
    }

    void generateControls()
    {
        QFile file(":/generatorconfig.json");
        if (!file.open(QIODevice::ReadOnly))
            throw std::runtime_error("Could not open file for reading: " + file.fileName().toStdString());

        QJsonDocument configDoc = QJsonDocument::fromJson(file.readAll());
        const auto rootObject = configDoc.object();
        const auto controlsArray = getArray("controls", rootObject);

        for (const auto controlValue : controlsArray) {
            const auto controlObj = controlValue.toObject();
            const QString name = getString("name", controlObj);
            try {
                generateControl(controlObj);
            } catch (std::exception &e) {
                qWarning().nospace().noquote() << "Could not generate control: " << e.what();
            }
        }
    }

    void generateControl(const QJsonObject controlObj)
    {
        const QString name = getString("name", controlObj);
        m_currentControl = name;
        debugHeader(name);

        // info from config document
        const auto atoms = getObject("atoms", controlObj);
        const auto componentSetName = getString("component set", controlObj);

        // info from figma document
        const auto documentRoot = getObject("document", m_document.object());
        const auto componentSet = findChild({"type", "COMPONENT_SET", "name", componentSetName}, documentRoot);

        const auto copy = getString("copy", controlObj);
        QStringList files = copy.split(',');
        for (const QString &file : files)
            copyFileToStyleFolder(file.trimmed(), false);

        // Add this control to the list of controls that goes into the qmldir file
        m_qmlDirControls.append(name);

        const auto states = getObject("states", controlObj);
        for (const QString &imageState : std::as_const(states).keys()) {
            try {
                // Resolve all atoms for the given state
                for (const QString &atomName : atoms.keys()) {
                    const auto atomPath = atoms[atomName].toString();
                    const auto figmaState = states[imageState].toString();
                    const auto atomInputConfig = getAtomConfig(atomPath);
                    const auto atom = getAtomObject(atomPath, figmaState, componentSet);
                    m_currentQualifiedAtomName = m_currentControl.toLower()
                        + "-" + atomName
                        + (imageState == "normal" ? "" : "-" + imageState);

                    generateAtomAssets(atomInputConfig, atom);
                }

                generatePaddingAndInsets(imageState);

            } catch (std::exception &e) {
                qWarning().nospace().noquote()
                    << "Warning: " << m_currentQualifiedAtomName << ": " << e.what();
            }
        }
    }

    QString getAtomConfig(const QString &path)
    {
        const int configPos = path.indexOf("(");
        if (configPos == -1)
            return {};
        return path.sliced(configPos);
    }

    QJsonObject getAtomObject(const QString &path, const QString &figmaState, const QJsonObject &componentSet)
    {
        QString pathWithoutConfig = path;
        const int configPos = path.indexOf("(");
        if (configPos != -1)
            pathWithoutConfig = path.first(configPos);

        // Construct the json search path to the atom component
        // inside the component set, and fetch it.
        QStringList jsonPath;
        QStringList atomPath = pathWithoutConfig.split(",");
        if (atomPath.isEmpty())
            throw NoChildFoundException("atom has no figma path!");
        jsonPath.append("state=" + figmaState);
        for (const QString &child : atomPath)
            jsonPath += child.trimmed();

        return findNamedChild(jsonPath, componentSet);
    }

    void generateAtomAssets(const QString &inputConfig, const QJsonObject &atom)
    {
        QJsonObject atomOutputConfig;
        atomOutputConfig.insert("figmaId", getString("id", atom));

        // Generate/export the atom assets according to the input config
        generateGeometry(inputConfig, atomOutputConfig, atom);
        if (inputConfig.contains("image"))
            generateImage(inputConfig, atomOutputConfig, atom);
        if (inputConfig.contains("json"))
            generateJson(inputConfig, atomOutputConfig, atom);

        // Add the atom configuration to the global config document
        m_outputConfig.insert(m_currentQualifiedAtomName, atomOutputConfig);
    }

    void generateGeometry(const QString &inputConfig, QJsonObject &outputConfig, const QJsonObject &atom)
    {
        const auto geometry = getGeometry(atom);
        const auto stretch = getStretch(atom);
        const bool is9p = inputConfig.contains("image9p");
        outputConfig.insert("x", geometry.x());
        outputConfig.insert("y", geometry.y());
        outputConfig.insert("width", geometry.width());
        outputConfig.insert("height", geometry.height());
        if (is9p && !stretch.horizontal.isEmpty()) {
            outputConfig.insert("horizontalStretch", stretch.horizontal);
            outputConfig.insert("verticalStretch", stretch.vertical);
        }

        // Todo: resolve insets for rectangles with drop shadow
        // config.insert("leftInset", 0);
        // config.insert("topInset", 0);
        // config.insert("rightInset", 0);
        // config.insert("bottomInset", 0);
    }

    void generatePaddingAndInsets(const QString &imageState)
    {
        // Generate padding and insets based on the background
        // and contents atoms, if available.
        const QString controlName = m_currentControl.toLower();
        const QString backgroundQualifiedName = controlName
            + "-background" + (imageState == "normal" ? "" : "-" + imageState);
        const QString contentsQualifiedName = controlName
            + "-contents" + (imageState == "normal" ? "" : "-" + imageState);

        auto bgConfig = m_outputConfig[backgroundQualifiedName].toObject();
        const auto contentsConfig = m_outputConfig[contentsQualifiedName].toObject();

        const auto bgGeo = getConfigGeometry(bgConfig);
        const auto coGeo = getConfigGeometry(contentsConfig);

        bgConfig.insert("leftPadding", coGeo.x());
        bgConfig.insert("topPadding", coGeo.y());
        bgConfig.insert("rightPadding", qMax(0., bgGeo.width() - coGeo.x() - coGeo.width()));
        bgConfig.insert("bottomPadding", qMax(0., bgGeo.height() - coGeo.y() - coGeo.height()));

        // Overwrite the previous config
        m_outputConfig.insert(backgroundQualifiedName, bgConfig);
    }

    void generateImage(const QString &inputConfig, QJsonObject &outputConfig, const QJsonObject &atom)
    {
        const QString figmaId = getString("id", atom);
        const bool is9p = inputConfig.contains("image9p");
        const QJsonValue visible = atom.value("visible");

        if (visible != QJsonValue::Undefined && !visible.toBool()) {
            // Figma will not generate an image for a hidden child
            debug("skipping hidden image: " + figmaId + ", " + m_currentQualifiedAtomName);
            return;
        }

        debug("export image: " + figmaId + ", " + m_currentQualifiedAtomName);
        m_imagesToDownload.insert(figmaId, m_currentQualifiedAtomName);
        addExportType(is9p ? "image9p" : "image", outputConfig);
    }

    void generateJson(const QString &inputConfig, QJsonObject &outputConfig, const QJsonObject &atom)
    {
        Q_UNUSED(inputConfig);
        const QString fileName = "json/" + m_currentQualifiedAtomName + ".json";
        createTextFileInStylefolder("export json", fileName, QJsonDocument(atom).toJson());
        addExportType("json", outputConfig);
    }

    void generateConfiguration()
    {
        QJsonObject root;
        root.insert("version", "1.0");
        root.insert("atoms", m_outputConfig);
        createTextFileInStylefolder("generating", "config.json", QJsonDocument(root).toJson());
    }

    void generateQmlDir()
    {
        const QString styleName = QFileInfo(m_targetPath).fileName();
        const QString version(" 1.0 ");

        QString qmldir;
        qmldir += "module " + styleName + "\n";
        for (const QString &control : m_qmlDirControls)
            qmldir += control + version + control + ".qml\n";

        createTextFileInStylefolder("generating", "qmldir", qmldir);
    }

    QJsonObject createConfiguration(const QJsonObject &background, const QJsonObject &contents)
    {
        const auto bgGeo = getGeometry(background);
        const auto coGeo = getGeometry(contents);
        const auto stretch = getStretch(background);

        QJsonObject config;
        config.insert("width", bgGeo.width());
        config.insert("height", bgGeo.height());
        config.insert("leftPadding", coGeo.x());
        config.insert("topPadding", coGeo.y());
        config.insert("rightPadding", qMax(0., bgGeo.width() - coGeo.x() - coGeo.width()));
        config.insert("bottomPadding", qMax(0., bgGeo.height() - coGeo.y() - coGeo.height()));
        config.insert("horizontalStretch", stretch.horizontal);
        config.insert("verticalStretch", stretch.vertical);

        // Todo: resolve insets for rectangles with drop shadow
        config.insert("leftInset", 0);
        config.insert("topInset", 0);
        config.insert("rightInset", 0);
        config.insert("bottomInset", 0);

        return config;
    }

    void addExportType(const QString &type, QJsonObject &outputConfig)
    {
        QString exportTypes = outputConfig.value("export").toString();
        if (!exportTypes.isEmpty())
            exportTypes += ",";
        outputConfig.insert("export", exportTypes + type);
    }

    void mkTargetPath(const QString &path) const
    {
        const QFileInfo fileInfo(path);
        if (fileInfo.exists())
            return;

        const QDir dir = fileInfo.absoluteDir();
        if (!QDir().mkpath(dir.path()))
            throw std::runtime_error("Could not create target path: " + dir.path().toStdString());
    }

    /**
     * Copies the given file into the style folder.
     * If destFileName is empty, the file name of the src will be used.
     */
    void copyFileToStyleFolder(const QString &srcPath, bool overwrite = true, QString destPath = "") const
    {
        QFile srcFile = QFile(srcPath);
        if (!srcFile.exists())
            throw std::runtime_error("File doesn't exist: " + srcPath.toStdString());
        if (destPath.isEmpty())
            destPath = QFileInfo(srcFile).fileName();

        QString targetPath = m_targetPath + "/" + destPath;
        mkTargetPath(targetPath);
        if (!overwrite && QFileInfo(targetPath).exists()) {
            debug(targetPath + " exists, skipping overwrite");
            return;
        }

        debug("copying " + QFileInfo(srcPath).fileName() + " to " + targetPath);
        srcFile.copy(targetPath);

        // Files we copy from resources are read-only, so change target permission
        // so that the user can modify generated QML files etc.
        QFile::setPermissions(targetPath, QFileDevice::ReadOwner | QFileDevice::ReadUser
            | QFileDevice::ReadGroup | QFileDevice::ReadOther | QFileDevice::WriteOwner);
    }

    void createTextFileInStylefolder(const QString &label, const QString &fileName, const QString &contents) const
    {
        const QString targetPath = m_targetPath + "/" + fileName;
        debug(label + ": " + targetPath);

        mkTargetPath(targetPath);

        QFile file(targetPath);
        if (!file.open(QIODevice::WriteOnly))
            throw std::runtime_error("Could not open file for writing: " + targetPath.toStdString());

        QTextStream out(&file);
        out << contents;
    }

    QRectF getGeometry(const QJsonObject figmaComponent, bool ignoreOrigin = true) const
    {
        const auto bb = getObject("absoluteBoundingBox", figmaComponent);
        const auto x = ignoreOrigin ? 0. : getValue("x", bb).toDouble();
        const auto y = ignoreOrigin ? 0. : getValue("y", bb).toDouble();
        const auto width = getValue("width", bb).toDouble();
        const auto height = getValue("height", bb).toDouble();
        return QRectF(x, y, width, height);
    }

    QRectF getConfigGeometry(const QJsonObject configAtom) const
    {
        // Read back the geometry we have already generated in the config object
        const auto x = getValue("x", configAtom).toDouble();
        const auto y = getValue("y", configAtom).toDouble();
        const auto width = getValue("width", configAtom).toDouble();
        const auto height = getValue("height", configAtom).toDouble();
        return QRectF(x, y, width, height);
    }

    Strectch9p getStretch(const QJsonObject rectangle) const
    {
        // Determine the stretch factor of the rectangle based on its corner radii
        qreal topLeftRadius = 0;
        qreal topRightRadius = 0;
        qreal bottomRightRadius = 0;
        qreal bottomLeftRadius = 0;

        const QJsonValue radiusValue = rectangle.value("cornerRadius");
        if (radiusValue != QJsonValue::Undefined) {
            const qreal r = radiusValue.toDouble();
            topLeftRadius = r;
            topRightRadius = r;
            bottomRightRadius = r;
            bottomLeftRadius = r;
        } else {
            const QJsonValue radiiValue = rectangle.value("rectangleCornerRadii");
            if (radiiValue == QJsonValue::Undefined)
                return {};
            const QJsonArray r = radiiValue.toArray();
            Q_ASSERT(r.count() == 4);
            topLeftRadius= r[0].toDouble();
            topRightRadius= r[1].toDouble();
            bottomRightRadius = r[2].toDouble();
            bottomLeftRadius = r[3].toDouble();
        }

        const qreal left = qMax(topLeftRadius, bottomLeftRadius);
        const qreal right = qMax(topRightRadius, bottomRightRadius);
        const qreal top = qMax(topLeftRadius, topRightRadius);
        const qreal bottom = qMax(bottomLeftRadius, bottomRightRadius);
        const QRectF geo = getGeometry(rectangle);

        return { {left, geo.width() - right}, {top, geo.height() - bottom} };
    }

    QString networkErrorString(QNetworkReply *reply)
    {
        return QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(reply->error());
    }

    void newProgress(const QString &label)
    {
        if (m_verbose || m_silent)
            return;
        m_progressLabel = label;
        m_progress = 0;
        printf("\33[2K%s: 0\r", qPrintable(m_progressLabel));
        fflush(stdout);
    }

    void progress()
    {
        if (m_verbose || m_silent)
            return;
        printf("\33[2K%s: %i\r", qPrintable(m_progressLabel), ++m_progress);
        fflush(stdout);
    }

    void debug(const QString &msg) const
    {
        if (!m_verbose)
            return;
        qDebug().noquote() << msg;
    }

    void debugHeader(const QString &msg) const
    {
        debug("");
        debug("*** " + msg + " ***");
    }

    void saveForDebug(const QJsonObject &object, const QString &name = "debug.json") const
    {
        createTextFileInStylefolder("debug", name, QJsonDocument(object).toJson());
    }

private:
    QString m_fileId;
    QString m_token;
    QString m_targetPath;
    bool m_verbose = false;
    bool m_silent = false;

    QJsonDocument m_document;
    QJsonObject m_outputConfig;
    QStringList m_qmlDirControls;
    QMap<QString, QString> m_imagesToDownload;

    QString m_currentControl;
    QString m_currentQualifiedAtomName;

    QString m_progressLabel;
    int m_progress = 0;
};

#endif // QSTYLEREADER_H
