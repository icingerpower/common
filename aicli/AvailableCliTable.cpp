#include "AvailableCliTable.h"

AvailableCliTable::AvailableCliTable(QObject *parent)
    : QAbstractTableModel(parent)
{
    for (AbstractCli *cli : AbstractCli::ALL_CLIS()) {
        m_rows.append({cli, false, false, {}});
    }
    refresh();
}

void AvailableCliTable::refresh()
{
    for (int row = 0; row < m_rows.size(); ++row) {
        m_rows[row].checked   = false;
        m_rows[row].available = false;
        m_rows[row].membership.clear();
        _startCheck(row);
    }
    if (!m_rows.isEmpty()) {
        emit dataChanged(index(0, ColAvailable),
                         index(m_rows.size() - 1, ColMembership),
                         {Qt::DisplayRole, Qt::CheckStateRole});
    }
}

void AvailableCliTable::_startCheck(int row)
{
    AbstractCli *cli = m_rows[row].cli;
    cli->checkAvailabilityAsync(this, [this, row](CliAvailability avail) {
        m_rows[row].checked    = true;
        m_rows[row].available  = avail.available;
        m_rows[row].membership = avail.membership;
        emit dataChanged(index(row, ColAvailable),
                         index(row, ColMembership),
                         {Qt::DisplayRole, Qt::CheckStateRole});
    });
}

AbstractCli *AvailableCliTable::cliAt(int row) const
{
    if (row < 0 || row >= m_rows.size()) {
        return nullptr;
    }
    return m_rows[row].cli;
}

// ---------------------------------------------------------------------------
// QAbstractTableModel
// ---------------------------------------------------------------------------

int AvailableCliTable::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rows.size();
}

int AvailableCliTable::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColCount;
}

QVariant AvailableCliTable::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    const int row = index.row();
    const int col = index.column();
    if (row < 0 || row >= m_rows.size()) {
        return {};
    }
    const RowData &r = m_rows[row];

    switch (col) {
    case ColName:
        if (role == Qt::DisplayRole) {
            return r.cli->getName();
        }
        if (role == Qt::ToolTipRole) {
            return r.cli->getDescription();
        }
        break;

    case ColAvailable:
        if (role == Qt::CheckStateRole) {
            if (!r.checked) {
                return Qt::PartiallyChecked; // check in progress
            }
            return r.available ? Qt::Checked : Qt::Unchecked;
        }
        if (role == Qt::ToolTipRole) {
            if (!r.checked) {
                return tr("Checking…");
            }
            return r.available ? tr("CLI found in PATH") : tr("CLI not found in PATH");
        }
        break;

    case ColMembership:
        if (role == Qt::DisplayRole) {
            if (!r.checked) {
                return tr("Checking…");
            }
            return r.membership;
        }
        break;

    default:
        break;
    }

    return {};
}

QVariant AvailableCliTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    switch (section) {
    case ColName:       return tr("CLI");
    case ColAvailable:  return tr("Available");
    case ColMembership: return tr("Membership");
    default:            return {};
    }
}

Qt::ItemFlags AvailableCliTable::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.column() == ColAvailable) {
        f |= Qt::ItemIsUserCheckable;
        f &= ~Qt::ItemIsEditable; // read-only: availability is detected, not set by the user
    }
    return f;
}
