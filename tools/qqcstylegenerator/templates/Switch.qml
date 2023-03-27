import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

T.Switch {
    id: control

    implicitWidth: leftInset + implicitBackgroundWidth + spacing + implicitContentWidth + rightPadding
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    spacing: 6

    topPadding: background ? background.topPadding : 0
    leftPadding: 122//background ? background.leftPadding : 0
    rightPadding: background ? background.rightPadding : 0
    bottomPadding: background ? background.bottomPadding : 0
    topInset: background ? -background.topInset || 0 : 0
    leftInset: background ? -background.leftInset || 0 : 0
    rightInset: contentItem.width + spacing
    bottomInset: background ? -background.bottomInset || 0 : 0

    indicator: Item {
        width: Math.max(handle.width, background.width)
        height: Math.max(handle.height, background.height)

        property NinePatchImage handle: NinePatchImage {
            parent: control.indicator
            x: Math.max(0, Math.min(parent.width - width, control.visualPosition * parent.width - (width / 2)))
            y: (parent.height - height) / 2

            Behavior on x {
                enabled: !control.down
                SmoothedAnimation {
                    velocity: 200
                }
            }

            source: Qt.resolvedUrl("images/switch-handle")
            NinePatchImageSelector on source {
                states: [
                    {"disabled": !control.enabled},
                    {"pressed": control.down},
                    {"checked": control.checked},
                    {"focused": control.visualFocus},
                    {"mirrored": control.mirrored},
                    {"hovered": control.enabled && control.hovered}
                ]
            }

            Image {
                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                source: Qt.resolvedUrl("images/switch-handle")
                ImageSelector on source {
                    states: [
                        {"righticon": control.checked},
                        {"lefticon": !control.checked},
                        {"disabled": !control.enabled},
                        {"pressed": control.down},
                        {"checked": control.checked},
                        {"focused": control.visualFocus},
                        {"mirrored": control.mirrored},
                        {"hovered": control.enabled && control.hovered}
                    ]
                }
            }
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.palette.windowText
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    background: NinePatchImage {
        source: Qt.resolvedUrl("images/switch-background")
        NinePatchImageSelector on source {
            states: [
                {"disabled": !control.enabled},
                {"pressed": control.down},
                {"checked": control.checked},
                {"focused": control.visualFocus},
                {"mirrored": control.mirrored},
                {"hovered": control.enabled && control.hovered}
            ]
        }

        Image {
            // TODO: The exact position on the icon should be set from the json config file
            x: (control.indicator.handle.width - width) / 2
            y: (parent.height - height) / 2
            source: Qt.resolvedUrl("images/switch-background-lefticon")
            ImageSelector on source {
                states: [
                    {"disabled": !control.enabled},
                    {"pressed": control.down},
                    {"checked": control.checked},
                    {"focused": control.visualFocus},
                    {"mirrored": control.mirrored},
                    {"hovered": control.enabled && control.hovered}
                ]
            }
        }

        Image {
            // TODO: The exact position on the icon should be set from the json config file
            x: parent.width - control.indicator.handle.width + (control.indicator.handle.width / 2)
            y: (parent.height - height) / 2
            source: Qt.resolvedUrl("images/switch-background-righticon")
            ImageSelector on source {
                states: [
                    {"disabled": !control.enabled},
                    {"pressed": control.down},
                    {"checked": control.checked},
                    {"focused": control.visualFocus},
                    {"mirrored": control.mirrored},
                    {"hovered": control.enabled && control.hovered}
                ]
            }
        }
    }
}
