#pragma once

#include <QString>
#include <QStringList>

// Manages backups of the types.xml file
class BackupManager
{
public:
    explicit BackupManager(QObject *parent = nullptr);

    // Creates a backup of the file
    // Returns the backup path, or an empty string on failure
    QString createBackup(const QString &filePath);

    // Returns the list of backups for a file (newest first)
    QStringList listBackups(const QString &filePath) const;

    // Restores a backup to the target path
    bool restoreBackup(const QString &backupPath, const QString &targetPath);

    // Opens the backup folder in Explorer
    void openBackupFolder(const QString &filePath);

    // Maximum number of backups per file
    int maxBackups() const { return m_maxBackups; }
    void setMaxBackups(int n) { m_maxBackups = n; }

    // Auto-backup enabled
    bool autoBackupEnabled() const { return m_autoBackupEnabled; }
    void setAutoBackupEnabled(bool en) { m_autoBackupEnabled = en; }

    // Last error
    QString lastError() const { return m_lastError; }

private:
    int     m_maxBackups        = 20;
    bool    m_autoBackupEnabled = true;
    mutable QString m_lastError;

    // Backup folder path for a file
    QString backupDirFor(const QString &filePath) const;

    // Removes old backups that exceed the limit
    void cleanupOldBackups(const QString &backupDir, const QString &baseName);
};
