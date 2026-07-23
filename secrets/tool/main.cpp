// Secrets tool: self-test + one-time migration of legacy plaintext credentials
// into the OS keychain and (optionally) an encrypted cross-machine bundle.
//
//   secretstool selftest
//   secretstool migrate-amzbooks       <importer.ini>  [--bundle <file> --passphrase <pw>]
//   secretstool migrate-amazontemplate <settings.ini>  [--bundle <file> --passphrase <pw>]
//   secretstool list <AppName>
//
// Values are never printed; only key names and lengths.

#include <QCoreApplication>
#include <QStringList>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QMap>

#include "../SecretStore.h"
#include "../SecretBundle.h"
#include "../CredentialManager.h"

static QTextStream out(stdout);

// Minimal, robust INI reader: returns "Section/key" -> value with surrounding
// double quotes stripped. Handles backslashes in keys literally (QSettings does
// not), which the AmazonTemplate3 settings.ini relies on (e.g. eu\lwaRefreshToken).
static QMap<QString, QString> readIni(const QString &path)
{
    QMap<QString, QString> result;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return result;
    }
    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    QString section;
    while (!in.atEnd()) {
        QString line = in.readLine();
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith('#') || trimmed.startsWith(';')) {
            continue;
        }
        if (trimmed.startsWith('[') && trimmed.endsWith(']')) {
            section = trimmed.mid(1, trimmed.size() - 2);
            continue;
        }
        const int eq = line.indexOf('=');
        if (eq < 0) {
            continue;
        }
        QString key = line.left(eq).trimmed();
        QString val = line.mid(eq + 1).trimmed();
        if (val.size() >= 2 && val.startsWith('"') && val.endsWith('"')) {
            val = val.mid(1, val.size() - 2);
            val.replace(QStringLiteral("\\\""), QStringLiteral("\""));
            val.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
        }
        result.insert(section + '/' + key, val);
    }
    return result;
}

// Each pair maps a raw INI key (as spelled in the file) to the keychain account
// name the app will query (its logical settings key). For most apps these are
// identical; AmazonTemplate3 differs (INI backslashes vs logical forward slashes,
// and array-indexed Temu tokens).
static bool migrate(const QString &app, const QString &iniPath,
                    const QList<QPair<QString, QString>> &keyMap,
                    const QStringList &staleAccounts,
                    const QString &bundlePath, const QString &passphrase)
{
    if (!QFileInfo::exists(iniPath)) {
        out << "  ! ini not found: " << iniPath << "\n";
        return false;
    }
    const QMap<QString, QString> ini = readIni(iniPath);
    CredentialManager mgr(app);
    if (!mgr.storeAvailable()) {
        out << "  ! no Secret Service available on the session bus\n";
        return false;
    }

    // Remove any previously-stored accounts under wrong names.
    for (const QString &acc : staleAccounts) {
        QString err;
        mgr.store().removeSecret(acc, &err);
    }

    int migrated = 0;
    QStringList migratedKeys;   // account names actually stored
    QMap<QString, QString> rawByAccount;
    for (const auto &pair : keyMap) {
        const QString rawKey = pair.first;
        const QString account = pair.second;
        const QString val = ini.value(rawKey);
        if (val.isEmpty()) {
            out << "  - " << rawKey << ": absent in ini, skipped\n";
            continue;
        }
        QString err;
        if (mgr.set(account, val, &err)) {
            out << "  + " << account << " -> keychain (" << val.size() << " chars)\n";
            ++migrated;
            migratedKeys << account;
            rawByAccount.insert(account, val);
        } else {
            out << "  ! " << account << ": store failed: " << err << "\n";
        }
    }
    out << "  " << migrated << "/" << keyMap.size() << " secrets stored for " << app << "\n";

    // Verify read-back for each migrated account.
    int verified = 0;
    for (const QString &account : migratedKeys) {
        bool found = false;
        const QString v = mgr.get(account, &found);
        if (found && v == rawByAccount.value(account)) {
            ++verified;
        } else {
            out << "  ! verify FAILED for " << account << "\n";
        }
    }
    out << "  verified read-back: " << verified << "/" << migratedKeys.size() << "\n";

    if (!bundlePath.isEmpty() && !passphrase.isEmpty() && !migratedKeys.isEmpty()) {
        QString err;
        if (mgr.exportBundle(bundlePath, passphrase, migratedKeys, &err)) {
            out << "  bundle written: " << bundlePath << "\n";
            // Round-trip the bundle to prove the passphrase decrypts it.
            QMap<QString, QString> check;
            if (SecretBundle::importBundle(bundlePath, passphrase, &check, &err)
                && check.size() == migratedKeys.size()) {
                out << "  bundle verified (" << check.size() << " entries decrypt)\n";
            } else {
                out << "  ! bundle verify FAILED: " << err << "\n";
                return false;
            }
        } else {
            out << "  ! bundle export failed: " << err << "\n";
            return false;
        }
    }
    return verified == migratedKeys.size();
}

static int selftest()
{
    out << "== SecretBundle round-trip ==\n";
    QMap<QString, QString> in;
    in.insert(QStringLiteral("a/token"), QStringLiteral("héllo-éè-1234567890"));
    in.insert(QStringLiteral("b/secret"), QStringLiteral("{\"json\":[\"array\",\"of\",\"strings\"]}"));
    const QString tmp = QStringLiteral("/tmp/secretstool-selftest.enc");
    QString err;
    if (!SecretBundle::exportBundle(tmp, QStringLiteral("correct horse"), in, &err)) {
        out << "  export FAILED: " << err << "\n";
        return 1;
    }
    QMap<QString, QString> back;
    if (!SecretBundle::importBundle(tmp, QStringLiteral("correct horse"), &back, &err) || back != in) {
        out << "  import/compare FAILED: " << err << "\n";
        return 1;
    }
    out << "  OK: encrypt+decrypt matches\n";
    QMap<QString, QString> wrong;
    if (SecretBundle::importBundle(tmp, QStringLiteral("wrong passphrase"), &wrong, &err)) {
        out << "  FAILED: wrong passphrase accepted!\n";
        return 1;
    }
    out << "  OK: wrong passphrase rejected (" << err << ")\n";
    QFile::remove(tmp);

    out << "== SecretStore (live keyring) ==\n";
    SecretStore store(QStringLiteral("SecretsToolSelftest"));
    if (!store.isAvailable()) {
        out << "  ! no Secret Service on the session bus — cannot test keyring\n";
        return 2;
    }
    const QString acc = QStringLiteral("selftest/probe");
    const QString val = QStringLiteral("value-é-42");
    if (!store.setSecret(acc, val, &err)) {
        out << "  set FAILED: " << err << "\n";
        return 1;
    }
    bool found = false;
    const QString got = store.getSecret(acc, &found, &err);
    if (!found || got != val) {
        out << "  get FAILED (found=" << found << "): " << err << "\n";
        return 1;
    }
    out << "  OK: keychain set+get matches\n";
    store.removeSecret(acc, &err);
    found = true;
    store.getSecret(acc, &found, &err);
    out << (found ? "  ! remove FAILED\n" : "  OK: remove works\n");
    return found ? 1 : 0;
}

static QString optValue(const QStringList &args, const QString &name)
{
    const int i = args.indexOf(name);
    if (i >= 0 && i + 1 < args.size()) {
        return args.at(i + 1);
    }
    return QString();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const QStringList args = app.arguments();
    if (args.size() < 2) {
        out << "usage: secretstool selftest | migrate-amzbooks <ini> | "
               "migrate-amazontemplate <ini> | list <App>\n";
        return 1;
    }
    const QString cmd = args.at(1);

    if (cmd == QLatin1String("selftest")) {
        return selftest();
    }

    if (cmd == QLatin1String("export-bundle") && args.size() >= 5) {
        // export-bundle <App> <file> <passphrase>
        CredentialManager mgr(args.at(2));
        if (!mgr.storeAvailable()) {
            out << "no keychain available\n";
            return 1;
        }
        const QStringList accounts = mgr.store().listAccounts();
        QString err;
        if (mgr.exportBundle(args.at(3), args.at(4), accounts, &err)) {
            out << "wrote " << accounts.size() << " secret(s) to " << args.at(3) << "\n";
            return 0;
        }
        out << "export failed: " << err << "\n";
        return 1;
    }

    if (cmd == QLatin1String("import-bundle") && args.size() >= 5) {
        // import-bundle <App> <file> <passphrase>
        CredentialManager mgr(args.at(2));
        if (!mgr.storeAvailable()) {
            out << "no keychain available\n";
            return 1;
        }
        QString err;
        if (mgr.importBundle(args.at(3), args.at(4), &err)) {
            out << "imported bundle into the " << args.at(2) << " keychain\n";
            return 0;
        }
        out << "import failed: " << err << "\n";
        return 1;
    }

    if (cmd == QLatin1String("wipe") && args.size() >= 3) {
        // Remove every account of an application namespace (used to clean up
        // verification namespaces).
        SecretStore store(args.at(2));
        const QStringList accs = store.listAccounts();
        for (const QString &a : accs) {
            store.removeSecret(a);
        }
        out << "wiped " << accs.size() << " account(s) from " << args.at(2) << "\n";
        return 0;
    }

    if (cmd == QLatin1String("list") && args.size() >= 3) {
        SecretStore store(args.at(2));
        const QStringList accs = store.listAccounts();
        out << args.at(2) << ": " << accs.size() << " account(s)\n";
        for (const QString &a : accs) {
            out << "  " << a << "\n";
        }
        return 0;
    }

    if (cmd == QLatin1String("migrate-amzbooks") && args.size() >= 3) {
        // AmzBooks: INI key == keychain account (both use '/').
        const QList<QPair<QString, QString>> keyMap = {
            {QStringLiteral("Amazon SP-API (EU)/refreshToken"), QStringLiteral("Amazon SP-API (EU)/refreshToken")},
            {QStringLiteral("Amazon SP-API (EU)/clientSecret"), QStringLiteral("Amazon SP-API (EU)/clientSecret")},
            {QStringLiteral("Amazon SP-API (NA)/refreshToken"), QStringLiteral("Amazon SP-API (NA)/refreshToken")},
            {QStringLiteral("Amazon SP-API (NA)/clientSecret"), QStringLiteral("Amazon SP-API (NA)/clientSecret")},
            {QStringLiteral("Temu API/appSecrets"), QStringLiteral("Temu API/appSecrets")},
            {QStringLiteral("Temu API/accessTokens"), QStringLiteral("Temu API/accessTokens")},
        };
        out << "Migrating AmzBooks secrets from " << args.at(2) << "\n";
        const bool ok = migrate(QStringLiteral("AmzBooks"), args.at(2), keyMap, {},
                                optValue(args, QStringLiteral("--bundle")),
                                optValue(args, QStringLiteral("--passphrase")));
        return ok ? 0 : 1;
    }

    if (cmd == QLatin1String("migrate-amazontemplate") && args.size() >= 3) {
        // AmazonTemplate3: raw INI spelling (backslashes / array) -> logical
        // keychain account matching SettingsTable keys and the array read sites.
        const QList<QPair<QString, QString>> keyMap = {
            {QStringLiteral("AmazonApi/lwaClientSecret"),      QStringLiteral("AmazonApi/lwaClientSecret")},
            {QStringLiteral("AmazonApi/eu\\lwaRefreshToken"),  QStringLiteral("AmazonApi/eu/lwaRefreshToken")},
            {QStringLiteral("AmazonApi/na\\lwaRefreshToken"),  QStringLiteral("AmazonApi/na/lwaRefreshToken")},
            {QStringLiteral("AmazonApi/jp\\lwaRefreshToken"),  QStringLiteral("AmazonApi/jp/lwaRefreshToken")},
            {QStringLiteral("TemuApi/appSecret"),              QStringLiteral("TemuApi/appSecret")},
            {QStringLiteral("TemuApi/stores\\1\\token"),       QStringLiteral("TemuApi/stores/1/token")},
        };
        // Remove the wrongly-named accounts from the first migration pass.
        const QStringList stale = {
            QStringLiteral("AmazonApi/lwaRefreshToken"),
            QStringLiteral("AmazonApi/eu\\lwaRefreshToken"),
            QStringLiteral("AmazonApi/na\\lwaRefreshToken"),
            QStringLiteral("AmazonApi/jp\\lwaRefreshToken"),
            QStringLiteral("TemuApi/stores\\1\\token"),
        };
        out << "Migrating AmazonTemplate3 secrets from " << args.at(2) << "\n";
        const bool ok = migrate(QStringLiteral("AmazonTemplate3"), args.at(2), keyMap, stale,
                                optValue(args, QStringLiteral("--bundle")),
                                optValue(args, QStringLiteral("--passphrase")));
        return ok ? 0 : 1;
    }

    out << "unknown command\n";
    return 1;
}
