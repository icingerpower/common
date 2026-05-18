#include "AvailableCliList.h"
#include "AvailableCliTable.h"
#include "AbstractCli.h"

AvailableCliList::AvailableCliList(AvailableCliTable *source, QObject *parent)
    : QAbstractListModel(parent)
    , m_source(source)
{
    connect(m_source, &QAbstractItemModel::dataChanged,
            this, &AvailableCliList::_onSourceDataChanged);

    // Populate the initial filtered list (likely empty since checks are async).
    for (int r = 0; r < m_source->rowCount(); ++r) {
        if (_isAvailable(r)) {
            m_sourceRows.append(r);
        }
    }
}

bool AvailableCliList::_isAvailable(int sourceRow) const
{
    const QModelIndex idx = m_source->index(sourceRow, AvailableCliTable::ColAvailable);
    return m_source->data(idx, Qt::CheckStateRole).toInt() == Qt::Checked;
}

void AvailableCliList::_onSourceDataChanged(const QModelIndex &topLeft,
                                             const QModelIndex &bottomRight,
                                             const QList<int> &)
{
    // Only ColAvailable drives membership in this list.
    if (topLeft.column() > AvailableCliTable::ColAvailable ||
        bottomRight.column() < AvailableCliTable::ColAvailable) {
        return;
    }

    for (int sr = topLeft.row(); sr <= bottomRight.row(); ++sr) {
        const bool nowAvail = _isAvailable(sr);
        const int  myRow    = m_sourceRows.indexOf(sr);
        const bool wasAvail = (myRow >= 0);

        if (wasAvail && !nowAvail) {
            beginRemoveRows({}, myRow, myRow);
            m_sourceRows.removeAt(myRow);
            endRemoveRows();
        } else if (!wasAvail && nowAvail) {
            // Insert preserving ascending source-row order.
            int insertAt = 0;
            while (insertAt < m_sourceRows.size() && m_sourceRows[insertAt] < sr) {
                ++insertAt;
            }
            beginInsertRows({}, insertAt, insertAt);
            m_sourceRows.insert(insertAt, sr);
            endInsertRows();
        }
        // wasAvail && nowAvail: name/description can't change, nothing to emit.
    }
}

AbstractCli *AvailableCliList::cliAt(int row) const
{
    if (row < 0 || row >= m_sourceRows.size()) {
        return nullptr;
    }
    return m_source->cliAt(m_sourceRows[row]);
}

int AvailableCliList::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_sourceRows.size();
}

QVariant AvailableCliList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_sourceRows.size()) {
        return {};
    }
    AbstractCli *cli = m_source->cliAt(m_sourceRows[index.row()]);
    if (!cli) {
        return {};
    }
    switch (role) {
    case Qt::DisplayRole:  return cli->getName();
    case Qt::ToolTipRole:  return cli->getDescription();
    case CliPtrRole:       return QVariant::fromValue(cli);
    default:               return {};
    }
}
