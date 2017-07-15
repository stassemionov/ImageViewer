#ifndef HISTORY_H
#define HISTORY_H

#include <QSharedPointer>
#include <QImage>
#include <QList>

class EditHistory
{
public:
    EditHistory(const QString& filename, int max_size = 10);
    EditHistory(const QString& filename,
                const QString& storage_directory, int max_size = 10);
    ~EditHistory();

    // Set name of file, that is edited.
    void setFileName(const QString& filename);
    // Get name of file, that is edited.
    QString getFileName();
    // Set directory, that stores history files.
    bool setStoreDirectory(const QString& dir);
    // Get directory, that stores history files.
    QString getStoreDirectory();
    // Set maximal count of records, that can be stored in main memory.
    bool setMaxStoredCount(int count);
    // Get maximal count of records, that can be stored in main memory.
    int getMaxStoredCount();

    // Total count of stored records (in memory and on disk).
    int getHistoryLenght();

    // Create new record with a copy of given image.
    void add(const QImage& image);
    // Getting copy of more older record of history.
    QSharedPointer<QImage> back();
    // Getting copy of more newer record of history.
    QSharedPointer<QImage> forward();
    // Remove all stored images from hard disk and main memory.
    void clean();

    // *** Additional functionality:

    // Check if current version is oroginal.
    bool isAtStart();
    // Check if current version is latest version.
    bool isAtEnd();
    // Jump to version with specified index.
    bool jumpToVersion(int index);
    // Get copy of latest version, that is stored in history.
    QSharedPointer<QImage> getLatestVersion();
    // Get copy of current version.
    QSharedPointer<QImage> getCurrentVersion();
    // Get mostly earlyer version (original).
    QSharedPointer<QImage> getOriginal();
    // Get index of current version.
    int getCurrentIndex();

protected:
    // Removes outdated images from memory and disk.
    void removeOutdated();
    // Get copy of image that is pointed by current pointer.
    QSharedPointer<QImage> get();

private:
    // Name of the file, for which history was created.
    QString m_filename;
    // Maximum count of images that can be stored in main memory.
    int m_max_local_storage_size = 10;
    // Lenght of whole history.
    int m_total_stored_count = 0;
    // List of images that are stored in main memory.
    QList<QImage*> m_list;
    // Directory to create folder to store history data.
    QString m_storage_dir;
    // Index of currently used image.
    int m_local_pointer = -1;
    // Virtual shift in history to images that areasored in main memory.
    int m_shift = 0;
};

#endif // HISTORY_H
