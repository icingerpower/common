#ifndef SECRETSTORE_H
#define SECRETSTORE_H

#include <QString>
#include <QStringList>

// Thin client for the freedesktop.org Secret Service (org.freedesktop.secrets),
// implemented directly over QtDBus so it needs no external library. On Linux
// desktops this is the same vault qtkeychain uses — gnome-keyring or KWallet.
//
// Secrets are stored in the default collection with attributes
// {application, account}, so each app namespaces its own entries. Values are
// UTF-8 strings (API tokens, client secrets…).
//
// isAvailable() is false when no Secret Service is on the session bus (e.g. a
// headless run with no keyring daemon); callers should fall back to an explicit
// credential source rather than assuming storage succeeded.
class SecretStore
{
public:
    explicit SecretStore(const QString &application);

    bool isAvailable() const;

    // Stores (or replaces) the secret for `account`. Returns false on error.
    bool setSecret(const QString &account, const QString &secret, QString *error = nullptr);

    // Returns the stored secret; sets *found=false when the account is absent.
    QString getSecret(const QString &account, bool *found = nullptr, QString *error = nullptr);

    bool removeSecret(const QString &account, QString *error = nullptr);

    // Accounts stored by this application.
    QStringList listAccounts(QString *error = nullptr);

private:
    QString m_application;

    // Opens a plaintext transport session; returns the session object path
    // ("" on failure). The transport is plaintext because the D-Bus peer is the
    // local keyring daemon over the user's private session bus.
    QString openSession(QString *error) const;

    // Ensures the given collection/item object paths are unlocked, driving a
    // Prompt if the daemon requires one. Returns false if still locked.
    bool ensureUnlocked(const QStringList &objectPaths, QString *error) const;

    QString defaultCollectionPath() const;
};

#endif // SECRETSTORE_H
