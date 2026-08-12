#include "BackupManager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <algorithm>

BackupManager::BackupManager(QObject * /*parent*/)
{
}

// ────────────────────────────────────────────────────────────
// backupDirFor — returns the backups/ folder for a file
// ────────────────────────────────────────────────────────────
QString BackupManager::backupDirFor(const QString &filePath) const
{
    QFileInfo fi(filePath);
    return fi.absoluteDir().filePath("backups");
}

// ────────────────────────────────────────────────────────────
// createBackup — creates a backup copy
// ────────────────────────────────────────────────────────────
QString BackupManager::createBackup(const QString &filePath)
{
    m_lastError.clear();

    QFileInfo fi(filePath);
    if (!fi.exists()) {
        m_lastError = QString("Source file does not exist: %1").arg(filePath);
        return {};
    }

    const QString backupDir = backupDirFor(filePath);
    QDir dir;
    if (!dir.mkpath(backupDir)) {
        m_lastError = QString("Cannot create backup directory: %1").arg(backupDir);
        return {};
    }

    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    const QString baseName  = fi.completeBaseName(); // e.g. "types"
    const QString ext       = fi.suffix();            // e.g. "xml"
    const QString backupName = QString("%1_%2.%3").arg(baseName, timestamp, ext);
    const QString backupPath = QDir(backupDir).filePath(backupName);

    if (!QFile::copy(filePath, backupPath)) {
        m_lastError = QString("Failed to copy file to backup: %1").arg(backupPath);
        return {};
    }

    // Cleanup of old backups
    cleanupOldBackups(backupDir, baseName);

    return backupPath;
}

// ────────────────────────────────────────────────────────────
// listBackups — returns the backup list (newest first)
// ────────────────────────────────────────────────────────────
QStringList BackupManager::listBackups(const QString &filePath) const
{
    QFileInfo fi(filePath);
    const QString backupDir = backupDirFor(filePath);
    const QString baseName  = fi.completeBaseName();

    QDir dir(backupDir);
    if (!dir.exists())
        return {};

    // Filter files belonging to this base name
    QStringList filter;
    filter << QString("%1_*.%2").arg(baseName, fi.suffix());

    QFileInfoList entries = dir.entryInfoList(filter, QDir::Files, QDir::Time);

    QStringList result;
    for (const QFileInfo &entry : entries)
        result << entry.absoluteFilePath();

    return result;
}

// ────────────────────────────────────────────────────────────
// restoreBackup — restores a backup to the target
// ────────────────────────────────────────────────────────────
bool BackupManager::restoreBackup(const QString &backupPath, const QString &targetPath)
{
    m_lastError.clear();

    if (!QFile::exists(backupPath)) {
        m_lastError = QString("Backup file not found: %1").arg(backupPath);
        return false;
    }

    // Remove the existing file
    if (QFile::exists(targetPath)) {
        if (!QFile::remove(targetPath)) {
            m_lastError = QString("Cannot remove existing file: %1").arg(targetPath);
            return false;
        }
    }

    if (!QFile::copy(backupPath, targetPath)) {
        m_lastError = QString("Failed to restore backup to: %1").arg(targetPath);
        return false;
    }

    return true;
}

// ────────────────────────────────────────────────────────────
// openBackupFolder — opens the backup folder in Explorer
// ────────────────────────────────────────────────────────────
void BackupManager::openBackupFolder(const QString &filePath)
{
    const QString backupDir = backupDirFor(filePath);
    QDir().mkpath(backupDir);
    QDesktopServices::openUrl(QUrl::fromLocalFile(backupDir));
}

// ────────────────────────────────────────────────────────────
// cleanupOldBackups — keeps only the newest m_maxBackups
// ────────────────────────────────────────────────────────────
void BackupManager::cleanupOldBackups(const QString &backupDir, const QString &baseName)
{
    QDir dir(backupDir);
    QStringList filter;
    filter << QString("%1_*.xml").arg(baseName);

    // Sort: newest first
    QFileInfoList entries = dir.entryInfoList(filter, QDir::Files, QDir::Time);

    while (entries.size() > m_maxBackups) {
        QFileInfo oldest = entries.takeLast();
        QFile::remove(oldest.absoluteFilePath());
    }
}
