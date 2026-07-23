#include "SecretBundle.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>

namespace {

const QByteArray kMagic = QByteArrayLiteral("AMZSEC01");
constexpr int kSaltLen = 16;
constexpr int kIvLen   = 12;   // GCM standard nonce
constexpr int kTagLen  = 16;
constexpr int kKeyLen  = 32;   // AES-256

// scrypt work factors: N=2^15 (~32 MiB), r=8, p=1 — interactive-login strength.
constexpr uint64_t kScryptN = 1u << 15;
constexpr uint64_t kScryptR = 8;
constexpr uint64_t kScryptP = 1;
constexpr uint64_t kScryptMaxMem = 64ull * 1024 * 1024;

bool deriveKey(const QString &passphrase, const QByteArray &salt, QByteArray *keyOut)
{
    const QByteArray pass = passphrase.toUtf8();
    keyOut->resize(kKeyLen);
    // EVP_PBE_scrypt handles N/r/p directly and is available since OpenSSL 1.1.
    const int rc = EVP_PBE_scrypt(
        pass.constData(), static_cast<size_t>(pass.size()),
        reinterpret_cast<const unsigned char *>(salt.constData()), static_cast<size_t>(salt.size()),
        kScryptN, kScryptR, kScryptP, kScryptMaxMem,
        reinterpret_cast<unsigned char *>(keyOut->data()), static_cast<size_t>(keyOut->size()));
    return rc == 1;
}

} // namespace

bool SecretBundle::exportBundle(const QString &filePath,
                                const QString &passphrase,
                                const QMap<QString, QString> &secrets,
                                QString *error)
{
    auto fail = [&error](const QString &msg) {
        if (error) {
            *error = msg;
        }
        return false;
    };

    if (passphrase.isEmpty()) {
        return fail(QStringLiteral("Passphrase cannot be empty"));
    }

    QJsonObject obj;
    for (auto it = secrets.cbegin(); it != secrets.cend(); ++it) {
        obj.insert(it.key(), it.value());
    }
    const QByteArray plaintext = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    QByteArray salt(kSaltLen, '\0');
    QByteArray iv(kIvLen, '\0');
    if (RAND_bytes(reinterpret_cast<unsigned char *>(salt.data()), kSaltLen) != 1
        || RAND_bytes(reinterpret_cast<unsigned char *>(iv.data()), kIvLen) != 1) {
        return fail(QStringLiteral("Failed to gather random bytes"));
    }

    QByteArray key;
    if (!deriveKey(passphrase, salt, &key)) {
        return fail(QStringLiteral("Key derivation (scrypt) failed"));
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return fail(QStringLiteral("Cipher context allocation failed"));
    }

    QByteArray ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH, '\0');
    QByteArray tag(kTagLen, '\0');
    bool ok = false;
    do {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
            break;
        }
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, kIvLen, nullptr) != 1) {
            break;
        }
        if (EVP_EncryptInit_ex(ctx, nullptr, nullptr,
                               reinterpret_cast<const unsigned char *>(key.constData()),
                               reinterpret_cast<const unsigned char *>(iv.constData())) != 1) {
            break;
        }
        int len = 0;
        int total = 0;
        if (EVP_EncryptUpdate(ctx,
                              reinterpret_cast<unsigned char *>(ciphertext.data()), &len,
                              reinterpret_cast<const unsigned char *>(plaintext.constData()),
                              plaintext.size()) != 1) {
            break;
        }
        total = len;
        if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(ciphertext.data()) + total, &len) != 1) {
            break;
        }
        total += len;
        ciphertext.resize(total);
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, kTagLen,
                                reinterpret_cast<unsigned char *>(tag.data())) != 1) {
            break;
        }
        ok = true;
    } while (false);
    EVP_CIPHER_CTX_free(ctx);

    if (!ok) {
        return fail(QStringLiteral("AES-256-GCM encryption failed"));
    }

    QByteArray out;
    out.append(kMagic);
    out.append(salt);
    out.append(iv);
    out.append(tag);
    out.append(ciphertext);

    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return fail(QStringLiteral("Cannot open %1 for writing").arg(filePath));
    }
    if (f.write(out) != out.size()) {
        f.close();
        return fail(QStringLiteral("Short write to %1").arg(filePath));
    }
    f.close();
    // Owner-only permissions even though the content is encrypted.
    f.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    return true;
}

bool SecretBundle::importBundle(const QString &filePath,
                                const QString &passphrase,
                                QMap<QString, QString> *out,
                                QString *error)
{
    auto fail = [&error](const QString &msg) {
        if (error) {
            *error = msg;
        }
        return false;
    };

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        return fail(QStringLiteral("Cannot open %1").arg(filePath));
    }
    const QByteArray blob = f.readAll();
    f.close();

    const int headerLen = kMagic.size() + kSaltLen + kIvLen + kTagLen;
    if (blob.size() < headerLen || !blob.startsWith(kMagic)) {
        return fail(QStringLiteral("Not a valid secrets bundle"));
    }

    int off = kMagic.size();
    const QByteArray salt = blob.mid(off, kSaltLen); off += kSaltLen;
    const QByteArray iv   = blob.mid(off, kIvLen);   off += kIvLen;
    QByteArray tag        = blob.mid(off, kTagLen);  off += kTagLen;
    const QByteArray ciphertext = blob.mid(off);

    QByteArray key;
    if (!deriveKey(passphrase, salt, &key)) {
        return fail(QStringLiteral("Key derivation (scrypt) failed"));
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return fail(QStringLiteral("Cipher context allocation failed"));
    }

    QByteArray plaintext(ciphertext.size() + EVP_MAX_BLOCK_LENGTH, '\0');
    bool ok = false;
    do {
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
            break;
        }
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, kIvLen, nullptr) != 1) {
            break;
        }
        if (EVP_DecryptInit_ex(ctx, nullptr, nullptr,
                               reinterpret_cast<const unsigned char *>(key.constData()),
                               reinterpret_cast<const unsigned char *>(iv.constData())) != 1) {
            break;
        }
        int len = 0;
        int total = 0;
        if (EVP_DecryptUpdate(ctx,
                              reinterpret_cast<unsigned char *>(plaintext.data()), &len,
                              reinterpret_cast<const unsigned char *>(ciphertext.constData()),
                              ciphertext.size()) != 1) {
            break;
        }
        total = len;
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, kTagLen, tag.data()) != 1) {
            break;
        }
        // Returns <= 0 when the tag does not verify (wrong passphrase or tampering).
        if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(plaintext.data()) + total, &len) <= 0) {
            break;
        }
        total += len;
        plaintext.resize(total);
        ok = true;
    } while (false);
    EVP_CIPHER_CTX_free(ctx);

    if (!ok) {
        return fail(QStringLiteral("Wrong passphrase or corrupted bundle"));
    }

    QJsonParseError perr;
    const QJsonDocument doc = QJsonDocument::fromJson(plaintext, &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        return fail(QStringLiteral("Decrypted content is not valid"));
    }
    QMap<QString, QString> result;
    const QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        result.insert(it.key(), it.value().toString());
    }
    *out = result;
    return true;
}
