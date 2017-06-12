#ifndef HISTORY_H
#define HISTORY_H

#include <QList>
#include <QImage>

class EditHistory
{
public:
    EditHistory(const QString& filename, int size = 0);
    EditHistory(const QString& filename,
                const QString& store_directory, int size = 0);
    ~EditHistory();

    void setFileName(const QString& name);
    QString getFileName();
    bool setStoreDirectory(const QString& dir);
    bool setMaxStoredCount(int count);
    QString getStoreDirectory();
    int getMaxStoredCount();

    // Create new history record with a copy of given image.
    void add(const QImage& image);
    // Getting copy of more older record of history.
    QImage* back();
    // Getting copy of more newer record of history.
    QImage* forward();
    // Remove all stored images from hard disk and main memory.
    void clean();

protected:
    // Upload to hard disk all images starting with beginning of list
    // while size of list is greater than given max size.
    void uploadExcessImages();
    // Removes images from list starting with loc_pos.
    void removeUnused(int loc_pos);
    // Get copy of image that is pointed by current pointer.
    QImage* get();

private:
    // Name of the file, for which history was created.
    QString m_filename;
    // Maximum count of images that can be stored in main memory.
    int m_max_size = 10;
    // Count of stored images.
    int m_stored_count = 0;
    // The history data.
    QList<QImage*> m_list;
    // Directory to create folder to store history data.
    QString m_store_dir;
    // Index of currently used image.
    int m_pointer = -1;
};

#endif // HISTORY_H
