#ifndef AVAILABLECLILIST_H
#define AVAILABLECLILIST_H

#include <QAbstractListModel>
#include <QList>

class AvailableCliTable;
class AbstractCli;

// List model proxy over AvailableCliTable that exposes only confirmed-available
// CLIs (CheckStateRole == Qt::Checked). Updates live as async checks complete.
//
// Rows: one entry per available CLI, ordered by their position in the source table.
// Roles: DisplayRole → CLI name, ToolTipRole → description, CliPtrRole → AbstractCli*.
class AvailableCliList : public QAbstractListModel
{
    Q_OBJECT

public:
    static constexpr int CliPtrRole = Qt::UserRole + 1;

    explicit AvailableCliList(AvailableCliTable *source, QObject *parent = nullptr);

    // Returns the CLI pointer for the given row, or nullptr if out of range.
    AbstractCli *cliAt(int row) const;

    int      rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    void _onSourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                              const QList<int> &roles);
    bool _isAvailable(int sourceRow) const;

    AvailableCliTable *m_source;
    QList<int>         m_sourceRows; // source row indices currently passing the filter
};

#endif // AVAILABLECLILIST_H
