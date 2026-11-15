#ifndef CLIANTIGRAVITY_H
#define CLIANTIGRAVITY_H

#include "AbstractCli.h"

class CliAntigravity : public AbstractCli
{
public:
    explicit CliAntigravity(QString modelName = {}, QString displayName = {});

    QString getName() const override;
    QString getDescription() const override;
    QString getModel() const override;
    bool canGenSvg() const override;
    bool canGenImages() const override;
    bool canGenVideosFromText() const override;
    bool canGenVideoFromImages() const override;
    // The one CLI with a live web-search measurement (a 31-day daily-close
    // lookup answered in 1 m 52 s as one flat JSON object).
    bool canWebSearch() const override;
    QString getExecutable() const override;
    QStringList promptArgs() const override;
    // false: agy has no stdin prompt mode — `--print` takes the prompt text as
    // its flag value.  `--print -` would make the model answer the literal
    // prompt "-" while stdin is silently ignored.
    bool promptViaStdin() const override;
    QString preparePrompt(const QString &prompt) const override;
    CliErrorKind classifyError(const QString &errorOutput) const override;
    QStringList outputFallbackDirs() const override;

    // Parse the stdout of `agy models` ("<model-id>\t<Display Name>") into an
    // ordered, deduplicated list of human-readable model display names.
    static QStringList parseModelsOutput(const QString &stdoutText);

    // Query `agy models` asynchronously (coalescing concurrent callers into a
    // single QProcess), update ALL_CLIS() + the on-disk model cache when
    // non-empty, and notify `callback(liveSuccess)` if `context` is still alive.
    static void refreshModelsAsync(QObject *context,
                                   std::function<void(bool liveSuccess)> callback);

    // Synchronous `agy models` refresh used as a fallback when a headless CLI
    // invocation requests `--cli "Antigravity - <model>"` before the cache has
    // seen that model.
    static bool discoverModelsSync(int timeoutMs = 10000);

    // Override registered dynamic model variants in memory (without touching
    // the on-disk cache), intended for deterministic unit tests.
    static void setModelsForTesting(const QStringList &modelNames);

private:
    static const bool s_hooksRegistered;
    static void ensureInitialModelsLoaded();
    static bool resolveModelByCliName(const QString &nameOrExecutable);
    static void applyModelNames(const QStringList &modelNames);

    QString m_modelName;
    QString m_displayName;
};

#endif // CLIANTIGRAVITY_H
