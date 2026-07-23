#ifndef SECRETBUNDLE_H
#define SECRETBUNDLE_H

#include <QString>
#include <QMap>

// Portable, passphrase-protected export/import of a set of secrets, for moving
// credentials between machines (e.g. via a Dropbox-synced working directory).
//
// The on-disk file is authenticated encryption — safe to sync to a cloud drive:
//   magic "AMZSEC01" (8) | salt (16) | iv (12) | tag (16) | ciphertext
//   key   = scrypt(passphrase, salt, N=2^15, r=8, p=1) -> 32 bytes
//   cipher= AES-256-GCM over the compact-JSON map {account: secret}
//
// importBundle() returns false (without touching *out) when the passphrase is
// wrong or the file was tampered with — the GCM tag makes both indistinguishable
// from corruption, which is the desired behaviour.
class SecretBundle
{
public:
    // Encrypts `secrets` under `passphrase` and writes it to `filePath`.
    // On failure returns false and sets *error (if provided).
    static bool exportBundle(const QString &filePath,
                             const QString &passphrase,
                             const QMap<QString, QString> &secrets,
                             QString *error = nullptr);

    // Decrypts `filePath` with `passphrase` into *out. Returns false on a wrong
    // passphrase, a tampered/corrupt file, or I/O error (*error set, *out
    // untouched).
    static bool importBundle(const QString &filePath,
                             const QString &passphrase,
                             QMap<QString, QString> *out,
                             QString *error = nullptr);
};

#endif // SECRETBUNDLE_H
