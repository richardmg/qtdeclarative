/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQmlTreeModelToTableModel_H
#define QQmlTreeModelToTableModel_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtqmlmodelsglobal_p.h"

#include <QtCore/qset.h>
#include <QtCore/qpointer.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qitemselectionmodel.h>

QT_BEGIN_NAMESPACE

class QAbstractItemModel;

class Q_QMLMODELS_PRIVATE_EXPORT QQmlTreeModelToTableModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QModelIndex rootIndex READ rootIndex WRITE setRootIndex RESET resetRootIndex NOTIFY rootIndexChanged)

    struct TreeItem;

public:
    explicit QQmlTreeModelToTableModel(QObject *parent = nullptr);

    QAbstractItemModel *model() const;
    QModelIndex rootIndex() const;
    void setRootIndex(const QModelIndex &idx);
    void resetRootIndex();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    enum {
        DepthRole = Qt::UserRole - 5,
        ExpandedRole,
        HasChildrenRole,
        HasSiblingRole,
        ModelIndexRole
    };

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void clearModelData();

    bool isVisible(const QModelIndex &index);
    bool childrenVisible(const QModelIndex &index);

    QModelIndex mapToModel(const QModelIndex &index) const;
    QModelIndex mapFromModel(const QModelIndex &index) const;
    QModelIndex mapToModel(int row) const;

    Q_INVOKABLE QItemSelection selectionForRowRange(const QModelIndex &fromIndex, const QModelIndex &toIndex) const;

    void showModelTopLevelItems(bool doInsertRows = true);
    void showModelChildItems(const TreeItem &parent, int start, int end, bool doInsertRows = true, bool doExpandPendingRows = true);

    int itemIndex(const QModelIndex &index) const;
    void expandPendingRows(bool doInsertRows = true);
    int lastChildIndex(const QModelIndex &index) const;
    void removeVisibleRows(int startIndex, int endIndex, bool doRemoveRows = true);

    void dump() const;
    bool testConsistency(bool dumpOnFail = false) const;

    using QAbstractItemModel::hasChildren;

signals:
    void modelChanged(QAbstractItemModel *model);
    void rootIndexChanged();
    void expanded(const QModelIndex &index);
    void collapsed(const QModelIndex &index);

public slots:
    void expand(const QModelIndex &);
    void collapse(const QModelIndex &);
    void setModel(QAbstractItemModel *model);
    bool isExpanded(const QModelIndex &) const;
    bool isExpanded(int row) const;
    bool hasChildren(int row) const;
    bool hasSiblings(int row) const;
    int depthAtRow(int row) const;
    void expandRow(int n);
    void collapseRow(int n);

private slots:
    void modelHasBeenDestroyed();
    void modelHasBeenReset();
    void modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void modelLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint);
    void modelLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint);
    void modelRowsAboutToBeInserted(const QModelIndex & parent, int start, int end);
    void modelRowsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
    void modelRowsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
    void modelRowsInserted(const QModelIndex & parent, int start, int end);
    void modelRowsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
    void modelRowsRemoved(const QModelIndex & parent, int start, int end);

private:
    struct TreeItem {
        QPersistentModelIndex index;
        int depth;
        bool expanded;

        explicit TreeItem(const QModelIndex &idx = QModelIndex(), int d = 0, int e = false)
            : index(idx), depth(d), expanded(e)
        { }

        inline bool operator== (const TreeItem &other) const
        {
            return this->index == other.index;
        }
    };

    struct DataChangedParams {
        QModelIndex topLeft;
        QModelIndex bottomRight;
        QVector<int> roles;
    };

    struct SignalFreezer {
        SignalFreezer(QQmlTreeModelToTableModel *parent) : m_parent(parent) {
            m_parent->enableSignalAggregation();
        }
        ~SignalFreezer() { m_parent->disableSignalAggregation(); }

    private:
        QQmlTreeModelToTableModel *m_parent;
    };

    void enableSignalAggregation();
    void disableSignalAggregation();
    bool isAggregatingSignals() const { return m_signalAggregatorStack > 0; }
    void queueDataChanged(const QModelIndex &topLeft,
                          const QModelIndex &bottomRight,
                          const QVector<int> &roles);
    void emitQueuedSignals();

    QPointer<QAbstractItemModel> m_model = nullptr;
    QPersistentModelIndex m_rootIndex;
    QList<TreeItem> m_items;
    QSet<QPersistentModelIndex> m_expandedItems;
    QList<TreeItem> m_itemsToExpand;
    mutable int m_lastItemIndex = 0;
    bool m_visibleRowsMoved = false;
    int m_signalAggregatorStack = 0;
    QVector<DataChangedParams> m_queuedDataChanged;
    int m_column = 0;
};

QT_END_NAMESPACE

#endif // QQmlTreeModelToTableModel_H
