import QtQuick 2.0
import QmlBench 1.0
import QtQuick.Controls 2.2

CreationBenchmark {
    id: root
    count: 20
    staticCount: 1000
    delegate: DelayButton {
        x: QmlBench.getRandom() * root.width - width
        y: QmlBench.getRandom() * root.height - height
        text: "DelayButton"
        down: index % 2
    }
}
