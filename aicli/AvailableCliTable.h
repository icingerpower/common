#ifndef AVAILABLECLITABLE_H
#define AVAILABLECLITABLE_H

#include <QAbstractTableModel>
#include <QList>

#include "AbstractCli.h"

// Table model listing all registered AbstractCli subclasses with their
// availability status and membership/plan information.
//
// Columns:
//   0 — CLI name
//   1 — Available (shown as a checkbox: PartiallyChecked while checking)
//   2 — Membership (empty when not detectable)
//
// On construction the model immediately triggers async availability checks
// for every registered CLI. Each check updates its row and emits dataChanged
// so connected views refresh in real time.
class AvailableCliTable : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColName       = 0,
        ColAvailable  = 1,
        ColMembership = 2,
        ColCount      = 3,
    };

    explicit AvailableCliTable(QObject *parent = nullptr);

    // Restart availability checks for all rows (useful for a "Refresh" button).
    void refresh();

    // Returns the CLI for the given row, or nullptr if row is out of range.
    AbstractCli *cliAt(int row) const;

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    struct RowData {
        AbstractCli *cli      = nullptr;
        bool         checked  = false;  // true once the async check has returned
        bool         available = false;
        QString      membership;
    };

    void _startCheck(int row);

    QList<RowData> m_rows;
};

#endif // AVAILABLECLITABLE_H
