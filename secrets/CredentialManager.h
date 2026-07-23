#ifndef CREDENTIALMANAGER_H
#define CREDENTIALMANAGER_H

#include <QString>
#include <QStringList>
#include <QMap>

#include "SecretStore.h"

// Facade used by the apps and the migration tool. Wraps a SecretStore for one
// application and adds the cross-machine bundle import/export used by the
// first-run flow.
//
// Account keys are the same "group/key" paths the app used in its old ini file
// (e.g. "Amazon SP-API (EU)/refreshToken"), so integration is a drop-in swap of
// the read/write backend.
class CredentialManager
{
public:
    explicit CredentialManager(const QString &application);

    // Convenience for read sites migrating off plaintext settings: returns the
    // keychain secret for `account` if the store has it, otherwise `iniFallback`
    // (the value the caller already read from its legacy settings). Lets code
    // adopt the keychain without a hard cutover.
    static QString lookup(const QString &application, const QString &account,
                          const QString &iniFallback);

    bool storeAvailable() const;
    QString application() const { return m_application; }

    QString get(const QString &account, bool *found = nullptr) const;
    bool set(const QString &account, const QString &value, QString *error = nullptr);

    // True only if every listed account has a non-empty secret in the store.
    bool hasAll(const QStringList &accounts) const;

    // Writes the given accounts (read from the store) into an encrypted bundle.
    bool exportBundle(const QString &filePath, const QString &passphrase,
                      const QStringList &accounts, QString *error = nullptr);

    // Decrypts the bundle and writes every entry into the store.
    bool importBundle(const QString &filePath, const QString &passphrase,
                      QString *error = nullptr);

    SecretStore &store() { return m_store; }

private:
    QString m_application;
    SecretStore m_store;
};

#endif // CREDENTIALMANAGER_H
