#ifndef DELEGATEUECOUNTRIES_H
#define DELEGATEUECOUNTRIES_H

#include <qstyleditemdelegate.h>

class DelegateCountries : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DelegateCountries(
            std::function<bool(const QModelIndex &)> indexValid,
            QObject *parent = nullptr);
    QWidget *createEditor(
            QWidget *parent,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

    bool ueOnly() const;
    void setUeOnly(bool ueOnly);

protected:
    std::function<bool(const QModelIndex &)> m_indexValid;
    bool m_ueOnly;
};


#endif // DELEGATEUECOUNTRIES_H
