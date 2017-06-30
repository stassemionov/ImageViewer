#include <history.h>

#include <QApplication>
#include <QDir>

#include <QDebug>

EditHistory::EditHistory(const QString &filename, int max_size)
{
    m_filename = filename;
    if (max_size > 0)
    {
        m_max_local_storage_size = max_size;
    }
    m_storage_dir = QApplication::applicationDirPath();
    QString app_history_dir = QString::fromUtf8("app_history");
    QDir dir{m_storage_dir};
    if (!dir.cd(app_history_dir))
    {
        dir.mkdir(app_history_dir);
    }
    m_storage_dir += QString::fromUtf8("/") + app_history_dir;
}

EditHistory::EditHistory(const QString& filename,
                         const QString& storage_directory,
                         int max_size)
{
    m_filename = filename;
    if (max_size > 0)
    {
        m_max_local_storage_size = max_size;
    }
    if (storage_directory.isEmpty())
    {
        m_storage_dir = QApplication::applicationDirPath();
    }
    QDir dir{storage_directory};
    if (!dir.exists())
    {
        m_storage_dir = QApplication::applicationDirPath();
        dir.setPath(m_storage_dir);
    }
    QString app_history_dir = QString::fromUtf8("app_history");
    if (!dir.cd(app_history_dir))
    {
        dir.mkdir(app_history_dir);
    }
    m_storage_dir += QString::fromUtf8("/") + app_history_dir;
}

EditHistory::~EditHistory()
{
    this->clean();
    QDir dir{m_storage_dir};
    dir.removeRecursively();
}

void EditHistory::setFileName(const QString& filename)
{
    m_filename = filename;
}

QString EditHistory::getFileName()
{
    return m_filename;
}

bool EditHistory::setStoreDirectory(const QString& dir)
{
    QDir d(dir);
    if (d.exists())
    {
        m_storage_dir = dir;
        QString app_history_dir = QString::fromUtf8("app_history");
        if (!d.cd(app_history_dir))
        {
            d.mkdir(app_history_dir);
        }
        m_storage_dir += QString::fromUtf8("/") + app_history_dir;
        return true;
    }
    return false;
}

QString EditHistory::getStoreDirectory()
{
    int len = QString::fromUtf8("/app_history").length();
    return QString(m_storage_dir).remove(
                m_storage_dir.length() - len, len);
}

bool EditHistory::setMaxStoredCount(int count)
{
    if (count <= 0)
    {
        return false;
    }

    m_max_local_storage_size = count;

    if (count < m_list.size())
    {
        // Uploading of all excess images.
        // Firstly, we upload more newer versions.
        // Secondly, we upload more older versions.
        int count_to_delete = m_list.size() - count;
        while (m_list.size() != m_local_pointer + 1)
        {
            QImage* im = m_list.back();
            im->save(
                    QString::fromUtf8("%0/%1__%2").
                    arg(m_storage_dir).
                    arg(m_shift + m_list.size() - 1).
                    arg(m_filename),
                    0, 100);
            delete im;
            m_list.pop_back();
            --count_to_delete;
        }
        // If count of uploaded isn't enough, then
        // upload oldest versions, stored im main memory.
        while (count_to_delete != 0)
        {
            QImage* im = m_list.front();
            im->save(
                    QString::fromUtf8("%0/%1__%2").
                    arg(m_storage_dir).
                    arg(m_shift).
                    arg(m_filename),
                    0, 100);
            delete im;
            m_list.pop_front();
            ++m_shift;
            --m_local_pointer;
            --count_to_delete;
        }
    }
    return true;
}

int EditHistory::getMaxStoredCount()
{
    return m_max_local_storage_size;
}

int EditHistory::getHistoryLenght()
{
    return m_total_stored_count;
}

void EditHistory::add(const QImage& image)
{
    if (this->getHistoryLenght() > 1)
    {
        // clear all old images starting with next version.
        this->removeOutdated();
    }
    // If new record fits in allowed memory storage...
    if (m_local_pointer + 1 < m_max_local_storage_size)
    {
        // add of image to history.
        m_list.push_back(new QImage(image.copy()));
        // update size of history.
        m_total_stored_count = m_shift + m_list.size();
        // shift the pointer.
        ++m_local_pointer;
        return;
    }

    // ... else, if allowed memory storage is filled...

    // Upload and remove from memory the oldest image.
    QImage* image_to_upload = m_list[0];
    image_to_upload->save(
            QString::fromUtf8("%0/%1__%2").
            arg(m_storage_dir).
            arg(m_shift).
            arg(m_filename),
            0, 100);
    delete image_to_upload;
    m_list.pop_front();
    // Adding of new image.
    m_list.push_back(new QImage(image.copy()));
    // Updating of history parameters.
    ++m_shift;
    m_total_stored_count = m_shift + m_list.size();
}

QSharedPointer<QImage> EditHistory::back()
{
    if ((m_shift + m_local_pointer) < 1)
    {
        return QSharedPointer<QImage>();
    }

    if (m_local_pointer != 0)
    {
        --m_local_pointer;
        return QSharedPointer<QImage>(
                new QImage(m_list[m_local_pointer]->copy()));
    }
    else
    {
        QImage* loaded_image = new QImage(
            QString::fromUtf8("%0/%1__%2").
            arg(m_storage_dir).
            arg(m_shift - 1).
            arg(m_filename));

        if (m_list.size() == m_max_local_storage_size)
        {
            QImage* image_to_upload = m_list.back();
            image_to_upload->save(
                    QString::fromUtf8("%0/%1__%2").
                    arg(m_storage_dir).
                    arg(m_shift + m_list.size() - 1).
                    arg(m_filename),
                    0, 100);
            delete image_to_upload;
            m_list.pop_back();
        }
        m_list.push_front(loaded_image);

        --m_shift;

        return QSharedPointer<QImage>(
                new QImage(loaded_image->copy()));
    }
}

QSharedPointer<QImage> EditHistory::forward()
{
    if ((m_shift + m_local_pointer) == (m_total_stored_count - 1))
    {
        return QSharedPointer<QImage>();
    }

    if (m_local_pointer != (m_list.size() - 1))
    {
        ++m_local_pointer;
        return QSharedPointer<QImage>(
                new QImage(m_list[m_local_pointer]->copy()));
    }
    else
    {
        QImage* loaded_image = new QImage(
            QString::fromUtf8("%0/%1__%2").
            arg(m_storage_dir).
            arg(m_shift + m_list.size()).
            arg(m_filename));

        if (m_list.size() == m_max_local_storage_size)
        {
            QImage* image_to_upload = m_list.front();
            image_to_upload->save(
                    QString::fromUtf8("%0/%1__%2").
                    arg(m_storage_dir).
                    arg(m_shift).
                    arg(m_filename),
                    0, 100);
            delete image_to_upload;
            m_list.pop_front();
            ++m_shift;
        }
        m_list.push_back(loaded_image);

        return QSharedPointer<QImage>(
                new QImage(loaded_image->copy()));
    }
}

void EditHistory::clean()
{
    m_local_pointer = -1;
    m_shift = 0;
    this->removeOutdated();
    m_total_stored_count = 0;

    QDir dir{m_storage_dir};
    dir.removeRecursively();
    dir.cdUp();
    dir.mkdir(QString::fromUtf8("app_history"));
    dir.cd(QString::fromUtf8("app_history"));
}

void EditHistory::removeOutdated()
{
    // Current version is up-to-date.
    if (((m_shift + m_local_pointer + 1) == m_total_stored_count) ||
        m_list.isEmpty())
    {
        return;
    }
    // Removing from the disk.
    QDir dir(m_storage_dir);
    int first_disk_image_index = m_shift + m_local_pointer + 1;
    for (int i = first_disk_image_index;
         i < m_total_stored_count; ++i)
    {
        dir.remove(QString::fromUtf8("%0__%1").
            arg(i).arg(m_filename));
    }
    // Removing from main memory.
    if (m_local_pointer < m_list.size() - 1)
    {
        while (m_list.size() != (m_local_pointer + 1))
        {
            delete m_list.back();
            m_list.pop_back();
        }
    }
    m_total_stored_count = m_shift + m_local_pointer + 1;
}

bool EditHistory::isAtStart()
{
    if (m_list.isEmpty())
    {
        return false;
    }
    return (m_shift + m_local_pointer) == 0;
}

bool EditHistory::isAtEnd()
{
    if (m_list.isEmpty())
    {
        return false;
    }
    return (m_shift + m_local_pointer + 1) == m_total_stored_count;
}

bool EditHistory::jumpToVersion(int index)
{
    // Check if index is incorrect or out of range.
    if ((index < 0) ||
        (index >= m_total_stored_count) ||
        (index == (m_shift + m_local_pointer)))
    {
        return false;
    }
    // New index is in list of images stored in memory.
    if ((index >= m_shift) && (index < (m_shift + m_list.size())))
    {
        // Just update a pointer.
        m_local_pointer = index - m_shift;
    }
    else // New index is out of list of images stored in memory.
    {
        // Uploading of all memory-stored images list.
        for (int i = 0; i < m_list.size(); ++i)
        {
            m_list[i]->save(
                QString::fromUtf8("%0/%1__%2").
                arg(m_storage_dir).
                arg(m_shift + i).
                arg(m_filename),
                0, 100);
            delete m_list[i];
        }
        m_list.clear();
        // Creating of new list with one image.
        m_list.push_back(new QImage(
            QString::fromUtf8("%0/%1__%2").
            arg(m_storage_dir).
            arg(index).
            arg(m_filename)));
        // Updating of history parameters.
        m_shift = index;
        m_local_pointer = 0;
    }
    return true;
}

QSharedPointer<QImage> EditHistory::getLatestVersion()
{
    // History is empty.
    if (m_list.isEmpty())
    {
        return QSharedPointer<QImage>();
    }

    if ((m_shift + m_list.size()) == m_total_stored_count)
    {
        return QSharedPointer<QImage>(
                new QImage(QString::fromUtf8("%0/%1__%2").
                    arg(m_storage_dir).
                    arg(m_total_stored_count - 1).
                    arg(m_filename)));
    }
    else
    {
        return QSharedPointer<QImage>(
                new QImage(m_list.back()->copy()));
    }
}

QSharedPointer<QImage> EditHistory::getCurrentVersion()
{
    if (m_local_pointer < 0)
    {
        return QSharedPointer<QImage>();
    }
    return QSharedPointer<QImage>(
            new QImage(m_list[m_local_pointer]->copy()));
}

int EditHistory::getCurrentIndex()
{
    return m_shift + m_local_pointer;
}
