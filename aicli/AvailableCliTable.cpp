#include "AvailableCliTable.h"
#include "CliAntigravity.h"

#include <QVector>

QString AvailableCliTable::_execCheckKey(const AbstractCli *cli)
{
    return cli->getExecutable() + QLatin1Char('\x1f')
           + cli->availabilityArgs().join(QLatin1Char('\x1f'));
}

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
    ++m_refreshGeneration;
    const quint64 generation = m_refreshGeneration;
    m_inFlightExecKeys.clear();
    m_resolvedExecAvail.clear();

    syncWithRegistry(false);

    for (RowData &row : m_rows) {
        row.checked   = false;
        row.available = false;
        row.membership.clear();
    }
    if (!m_rows.isEmpty()) {
        emit dataChanged(index(0, ColAvailable),
                         index(m_rows.size() - 1, ColMembership),
                         {Qt::DisplayRole, Qt::CheckStateRole});
    }

    // Coalesce `--version` checks by executable + availabilityArgs so 20+
    // Antigravity model variants do not spawn 20+ duplicate subprocesses.
    for (const RowData &row : m_rows) {
        _ensureExecCheckStarted(row.cli, generation);
    }

    // Also query `agy models` asynchronously so newly released models (such as
    // Hydrogen / Carbon) appear automatically without hardcoding.
    CliAntigravity::refreshModelsAsync(this, [this, generation](bool liveSuccess) {
        if (generation != m_refreshGeneration) {
            return;
        }
        syncWithRegistry(liveSuccess);
    });
}

void AvailableCliTable::_ensureExecCheckStarted(AbstractCli *cli, quint64 generation)
{
    if (!cli) {
        return;
    }
    const QString key = _execCheckKey(cli);
    if (m_inFlightExecKeys.contains(key) || m_resolvedExecAvail.contains(key)) {
        return;
    }
    m_inFlightExecKeys.insert(key);

    cli->checkAvailabilityAsync(this, [this, key, generation](CliAvailability avail) {
        if (generation != m_refreshGeneration) {
            return;
        }
        m_inFlightExecKeys.remove(key);
        m_resolvedExecAvail.insert(key, avail);

        for (int row = 0; row < m_rows.size(); ++row) {
            if (_execCheckKey(m_rows[row].cli) != key) {
                continue;
            }
            m_rows[row].checked    = true;
            m_rows[row].available  = avail.available;
            m_rows[row].membership = m_rows[row].cli->parseMembership(avail.versionOutput);
            emit dataChanged(index(row, ColAvailable),
                             index(row, ColMembership),
                             {Qt::DisplayRole, Qt::CheckStateRole});
        }
    });
}

void AvailableCliTable::syncWithRegistry(bool antigravityLiveSuccess)
{
    const QList<AbstractCli *> &desiredClis = AbstractCli::ALL_CLIS();

    QHash<AbstractCli *, int> targetPos;
    targetPos.reserve(desiredClis.size());
    for (int i = 0; i < desiredClis.size(); ++i) {
        targetPos.insert(desiredClis.at(i), i);
    }

    const int n = m_rows.size();
    QVector<int> pos(n, -1);
    QHash<AbstractCli *, RowData> savedStates;
    savedStates.reserve(n);
    for (int i = 0; i < n; ++i) {
        pos[i] = targetPos.value(m_rows[i].cli, -1);
        savedStates.insert(m_rows[i].cli, m_rows[i]);
    }

    // Keep the longest increasing subsequence of existing rows so unchanged
    // items (and selection in connected views) stay untouched.
    QVector<int> dp(n, 0);
    QVector<int> prev(n, -1);
    int bestEnd = -1;
    int bestLen = 0;
    for (int i = 0; i < n; ++i) {
        if (pos[i] < 0) {
            continue;
        }
        dp[i] = 1;
        for (int j = 0; j < i; ++j) {
            if (pos[j] >= 0 && pos[j] < pos[i] && dp[j] + 1 > dp[i]) {
                dp[i]   = dp[j] + 1;
                prev[i] = j;
            }
        }
        if (dp[i] > bestLen) {
            bestLen = dp[i];
            bestEnd = i;
        }
    }

    QVector<bool> keep(n, false);
    for (int cur = bestEnd; cur >= 0; cur = prev[cur]) {
        keep[cur] = true;
    }

    for (int r = n - 1; r >= 0; --r) {
        if (!keep[r]) {
            beginRemoveRows({}, r, r);
            m_rows.removeAt(r);
            endRemoveRows();
        }
    }

    QList<AbstractCli *> needsCheck;
    for (int i = 0; i < desiredClis.size(); ++i) {
        AbstractCli *cli = desiredClis.at(i);
        if (i < m_rows.size() && m_rows.at(i).cli == cli) {
            continue;
        }

        RowData rd;
        rd.cli = cli;
        const QString key = _execCheckKey(cli);
        if (savedStates.contains(cli) && savedStates.value(cli).checked) {
            rd = savedStates.value(cli);
        } else if (m_resolvedExecAvail.contains(key)) {
            const CliAvailability &avail = m_resolvedExecAvail.value(key);
            rd.checked    = true;
            rd.available  = avail.available;
            rd.membership = cli->parseMembership(avail.versionOutput);
        } else if (antigravityLiveSuccess
                   && cli->getExecutable() == QStringLiteral("agy")) {
            rd.checked   = true;
            rd.available = true;
        } else {
            needsCheck.append(cli);
        }

        beginInsertRows({}, i, i);
        m_rows.insert(i, rd);
        endInsertRows();
    }

    if (antigravityLiveSuccess) {
        for (int r = 0; r < m_rows.size(); ++r) {
            if (!m_rows[r].checked
                && m_rows[r].cli->getExecutable() == QStringLiteral("agy")) {
                m_rows[r].checked   = true;
                m_rows[r].available = true;
                emit dataChanged(index(r, ColAvailable),
                                 index(r, ColMembership),
                                 {Qt::DisplayRole, Qt::CheckStateRole});
            }
        }
    }

    const quint64 generation = m_refreshGeneration;
    for (AbstractCli *cli : needsCheck) {
        _ensureExecCheckStarted(cli, generation);
    }
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
