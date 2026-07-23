#include "CredentialManager.h"
#include "SecretBundle.h"

CredentialManager::CredentialManager(const QString &application)
    : m_application(application)
    , m_store(application)
{
}

QString CredentialManager::lookup(const QString &application, const QString &account,
                                  const QString &iniFallback)
{
    SecretStore store(application);
    if (store.isAvailable()) {
        bool found = false;
        const QString v = store.getSecret(account, &found);
        if (found && !v.isEmpty()) {
            return v;
        }
    }
    return iniFallback;
}

bool CredentialManager::storeAvailable() const
{
    return m_store.isAvailable();
}

QString CredentialManager::get(const QString &account, bool *found) const
{
    // SecretStore::getSecret is not const (opens a session), but logically this
    // is a read; cast is confined here.
    return const_cast<SecretStore &>(m_store).getSecret(account, found);
}

bool CredentialManager::set(const QString &account, const QString &value, QString *error)
{
    return m_store.setSecret(account, value, error);
}

bool CredentialManager::hasAll(const QStringList &accounts) const
{
    for (const QString &acc : accounts) {
        bool found = false;
        const QString v = get(acc, &found);
        if (!found || v.isEmpty()) {
            return false;
        }
    }
    return true;
}

bool CredentialManager::exportBundle(const QString &filePath, const QString &passphrase,
                                     const QStringList &accounts, QString *error)
{
    QMap<QString, QString> secrets;
    for (const QString &acc : accounts) {
        bool found = false;
        const QString v = get(acc, &found);
        if (found && !v.isEmpty()) {
            secrets.insert(acc, v);
        }
    }
    if (secrets.isEmpty()) {
        if (error) {
            *error = QStringLiteral("No secrets to export");
        }
        return false;
    }
    return SecretBundle::exportBundle(filePath, passphrase, secrets, error);
}

bool CredentialManager::importBundle(const QString &filePath, const QString &passphrase,
                                     QString *error)
{
    QMap<QString, QString> secrets;
    if (!SecretBundle::importBundle(filePath, passphrase, &secrets, error)) {
        return false;
    }
    for (auto it = secrets.cbegin(); it != secrets.cend(); ++it) {
        if (!m_store.setSecret(it.key(), it.value(), error)) {
            return false;
        }
    }
    return true;
}
