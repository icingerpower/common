#include "SecretStore.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QDBusMetaType>
#include <QDBusVariant>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QEventLoop>
#include <QTimer>
#include <QMap>

namespace {

const QString kService   = QStringLiteral("org.freedesktop.secrets");
const QString kSvcPath   = QStringLiteral("/org/freedesktop/secrets");
const QString kSvcIface  = QStringLiteral("org.freedesktop.Secret.Service");
const QString kCollIface = QStringLiteral("org.freedesktop.Secret.Collection");
const QString kItemIface = QStringLiteral("org.freedesktop.Secret.Item");
const QString kPromptIface = QStringLiteral("org.freedesktop.Secret.Prompt");
const QString kDefaultCollection = QStringLiteral("/org/freedesktop/secrets/aliases/default");

// D-Bus struct for the Secret Service "Secret" type: (oayays).
struct SecretStruct {
    QDBusObjectPath session;
    QByteArray parameters;
    QByteArray value;
    QString contentType;
};

QDBusArgument &operator<<(QDBusArgument &arg, const SecretStruct &s)
{
    arg.beginStructure();
    arg << s.session << s.parameters << s.value << s.contentType;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, SecretStruct &s)
{
    arg.beginStructure();
    arg >> s.session >> s.parameters >> s.value >> s.contentType;
    arg.endStructure();
    return arg;
}

void registerTypes()
{
    static bool done = false;
    if (!done) {
        qDBusRegisterMetaType<SecretStruct>();
        // a{ss} — Secret Service item attributes; not a built-in D-Bus type.
        qDBusRegisterMetaType<QMap<QString, QString>>();
        done = true;
    }
}

} // namespace

Q_DECLARE_METATYPE(SecretStruct)

SecretStore::SecretStore(const QString &application)
    : m_application(application)
{
    registerTypes();
}

bool SecretStore::isAvailable() const
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        return false;
    }
    QDBusInterface dbus(QStringLiteral("org.freedesktop.DBus"),
                        QStringLiteral("/org/freedesktop/DBus"),
                        QStringLiteral("org.freedesktop.DBus"),
                        QDBusConnection::sessionBus());
    QDBusReply<QStringList> names = dbus.call(QStringLiteral("ListActivatableNames"));
    if (names.isValid() && names.value().contains(kService)) {
        return true;
    }
    QDBusReply<bool> owned = dbus.call(QStringLiteral("NameHasOwner"), kService);
    return owned.isValid() && owned.value();
}

QString SecretStore::openSession(QString *error) const
{
    QDBusInterface svc(kService, kSvcPath, kSvcIface, QDBusConnection::sessionBus());
    QDBusMessage reply = svc.call(QStringLiteral("OpenSession"),
                                  QStringLiteral("plain"),
                                  QVariant::fromValue(QDBusVariant(QString())));
    if (reply.type() == QDBusMessage::ErrorMessage) {
        if (error) {
            *error = reply.errorMessage();
        }
        return QString();
    }
    const QList<QVariant> args = reply.arguments();
    if (args.size() < 2) {
        if (error) {
            *error = QStringLiteral("Unexpected OpenSession reply");
        }
        return QString();
    }
    return args.at(1).value<QDBusObjectPath>().path();
}

QString SecretStore::defaultCollectionPath() const
{
    return kDefaultCollection;
}

bool SecretStore::ensureUnlocked(const QStringList &objectPaths, QString *error) const
{
    if (objectPaths.isEmpty()) {
        return true;
    }
    QList<QDBusObjectPath> objects;
    for (const QString &p : objectPaths) {
        objects.append(QDBusObjectPath(p));
    }

    QDBusInterface svc(kService, kSvcPath, kSvcIface, QDBusConnection::sessionBus());
    QDBusMessage reply = svc.call(QStringLiteral("Unlock"), QVariant::fromValue(objects));
    if (reply.type() == QDBusMessage::ErrorMessage) {
        if (error) {
            *error = reply.errorMessage();
        }
        return false;
    }
    const QList<QVariant> args = reply.arguments();
    if (args.size() < 2) {
        return true; // assume unlocked
    }
    const QString promptPath = args.at(1).value<QDBusObjectPath>().path();
    if (promptPath.isEmpty() || promptPath == QLatin1String("/")) {
        return true; // no prompt needed — already unlocked
    }

    // A prompt is required (collection locked): drive it and wait for Completed,
    // with a bounded fallback so a missing prompter can't hang us.
    QDBusConnection bus = QDBusConnection::sessionBus();
    QEventLoop loop;
    bus.connect(kService, promptPath, kPromptIface, QStringLiteral("Completed"),
                &loop, SLOT(quit()));
    QDBusInterface prompt(kService, promptPath, kPromptIface, bus);
    prompt.asyncCall(QStringLiteral("Prompt"), QString());
    QTimer::singleShot(60000, &loop, &QEventLoop::quit);
    loop.exec();

    // Re-query lock state by attempting a no-op Unlock again.
    QDBusMessage reply2 = svc.call(QStringLiteral("Unlock"), QVariant::fromValue(objects));
    if (reply2.type() == QDBusMessage::ErrorMessage) {
        if (error) {
            *error = reply2.errorMessage();
        }
        return false;
    }
    const QList<QVariant> a2 = reply2.arguments();
    if (a2.size() >= 2) {
        const QString p2 = a2.at(1).value<QDBusObjectPath>().path();
        if (!p2.isEmpty() && p2 != QLatin1String("/")) {
            if (error) {
                *error = QStringLiteral("Keyring is locked");
            }
            return false;
        }
    }
    return true;
}

bool SecretStore::setSecret(const QString &account, const QString &secret, QString *error)
{
    const QString session = openSession(error);
    if (session.isEmpty()) {
        return false;
    }
    if (!ensureUnlocked({defaultCollectionPath()}, error)) {
        return false;
    }

    QMap<QString, QString> attributes;
    attributes.insert(QStringLiteral("application"), m_application);
    attributes.insert(QStringLiteral("account"), account);

    QVariantMap properties;
    properties.insert(QStringLiteral("org.freedesktop.Secret.Item.Label"),
                      QStringLiteral("%1: %2").arg(m_application, account));
    properties.insert(QStringLiteral("org.freedesktop.Secret.Item.Attributes"),
                      QVariant::fromValue(attributes));

    SecretStruct sec;
    sec.session = QDBusObjectPath(session);
    sec.parameters = QByteArray();
    sec.value = secret.toUtf8();
    sec.contentType = QStringLiteral("text/plain; charset=utf8");

    QDBusInterface coll(kService, defaultCollectionPath(), kCollIface, QDBusConnection::sessionBus());
    QDBusMessage reply = coll.call(QStringLiteral("CreateItem"),
                                   QVariant::fromValue(properties),
                                   QVariant::fromValue(sec),
                                   true /* replace */);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        if (error) {
            *error = reply.errorMessage();
        }
        return false;
    }
    return true;
}

QString SecretStore::getSecret(const QString &account, bool *found, QString *error)
{
    if (found) {
        *found = false;
    }
    const QString session = openSession(error);
    if (session.isEmpty()) {
        return QString();
    }

    QMap<QString, QString> attributes;
    attributes.insert(QStringLiteral("application"), m_application);
    attributes.insert(QStringLiteral("account"), account);

    QDBusInterface svc(kService, kSvcPath, kSvcIface, QDBusConnection::sessionBus());
    QDBusMessage search = svc.call(QStringLiteral("SearchItems"), QVariant::fromValue(attributes));
    if (search.type() == QDBusMessage::ErrorMessage) {
        if (error) {
            *error = search.errorMessage();
        }
        return QString();
    }
    const QList<QVariant> sargs = search.arguments();
    QList<QDBusObjectPath> unlocked;
    QList<QDBusObjectPath> locked;
    if (!sargs.isEmpty()) {
        unlocked = qdbus_cast<QList<QDBusObjectPath>>(sargs.at(0));
    }
    if (sargs.size() > 1) {
        locked = qdbus_cast<QList<QDBusObjectPath>>(sargs.at(1));
    }

    QDBusObjectPath itemPath;
    if (!unlocked.isEmpty()) {
        itemPath = unlocked.first();
    } else if (!locked.isEmpty()) {
        if (!ensureUnlocked({locked.first().path()}, error)) {
            return QString();
        }
        itemPath = locked.first();
    } else {
        return QString(); // not found — *found stays false
    }

    QDBusInterface item(kService, itemPath.path(), kItemIface, QDBusConnection::sessionBus());
    QDBusMessage got = item.call(QStringLiteral("GetSecret"), QVariant::fromValue(QDBusObjectPath(session)));
    if (got.type() == QDBusMessage::ErrorMessage) {
        if (error) {
            *error = got.errorMessage();
        }
        return QString();
    }
    const QList<QVariant> gargs = got.arguments();
    if (gargs.isEmpty()) {
        return QString();
    }
    SecretStruct sec;
    const QDBusArgument arg = gargs.at(0).value<QDBusArgument>();
    arg >> sec;
    if (found) {
        *found = true;
    }
    return QString::fromUtf8(sec.value);
}

bool SecretStore::removeSecret(const QString &account, QString *error)
{
    openSession(error);
    QMap<QString, QString> attributes;
    attributes.insert(QStringLiteral("application"), m_application);
    attributes.insert(QStringLiteral("account"), account);

    QDBusInterface svc(kService, kSvcPath, kSvcIface, QDBusConnection::sessionBus());
    QDBusMessage search = svc.call(QStringLiteral("SearchItems"), QVariant::fromValue(attributes));
    if (search.type() == QDBusMessage::ErrorMessage) {
        if (error) {
            *error = search.errorMessage();
        }
        return false;
    }
    const QList<QVariant> sargs = search.arguments();
    QList<QDBusObjectPath> all;
    if (!sargs.isEmpty()) {
        all = qdbus_cast<QList<QDBusObjectPath>>(sargs.at(0));
    }
    if (sargs.size() > 1) {
        all += qdbus_cast<QList<QDBusObjectPath>>(sargs.at(1));
    }
    for (const QDBusObjectPath &p : all) {
        QDBusInterface item(kService, p.path(), kItemIface, QDBusConnection::sessionBus());
        item.call(QStringLiteral("Delete"));
    }
    return true;
}

QStringList SecretStore::listAccounts(QString *error)
{
    QMap<QString, QString> attributes;
    attributes.insert(QStringLiteral("application"), m_application);

    QDBusInterface svc(kService, kSvcPath, kSvcIface, QDBusConnection::sessionBus());
    QDBusMessage search = svc.call(QStringLiteral("SearchItems"), QVariant::fromValue(attributes));
    if (search.type() == QDBusMessage::ErrorMessage) {
        if (error) {
            *error = search.errorMessage();
        }
        return {};
    }
    const QList<QVariant> sargs = search.arguments();
    QList<QDBusObjectPath> all;
    if (!sargs.isEmpty()) {
        all = qdbus_cast<QList<QDBusObjectPath>>(sargs.at(0));
    }
    if (sargs.size() > 1) {
        all += qdbus_cast<QList<QDBusObjectPath>>(sargs.at(1));
    }

    QStringList accounts;
    for (const QDBusObjectPath &p : all) {
        // Read the Attributes (a{ss}) property via Properties.Get and demarshal
        // the QDBusArgument directly — QDBusInterface::property() cannot convert
        // an a{ss} property and would yield a QDBusRawType.
        QDBusMessage msg = QDBusMessage::createMethodCall(
            kService, p.path(), QStringLiteral("org.freedesktop.DBus.Properties"),
            QStringLiteral("Get"));
        msg << kItemIface << QStringLiteral("Attributes");
        const QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
        if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().isEmpty()) {
            continue;
        }
        const QDBusVariant dv = reply.arguments().first().value<QDBusVariant>();
        const QDBusArgument arg = dv.variant().value<QDBusArgument>();
        QMap<QString, QString> attrs;
        arg >> attrs;
        const QString acc = attrs.value(QStringLiteral("account"));
        if (!acc.isEmpty()) {
            accounts.append(acc);
        }
    }
    return accounts;
}
