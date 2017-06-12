#include <history.h>

#include <QApplication>
#include <QDir>

#include <QDebug>


EditHistory::EditHistory(const QString &filename, int size)
{
    m_filename = filename;
    if (size > 0)
    {
        m_max_size = size;
    }
    m_store_dir = QApplication::applicationDirPath();
    QString app_history_dir = QString::fromUtf8("app_history");
    QDir dir{m_store_dir};
    if (!dir.cd(app_history_dir))
    {
        dir.mkdir(app_history_dir);
    }
    m_store_dir += QString::fromUtf8("/") + app_history_dir;
}

EditHistory::EditHistory(const QString& filename,
                         const QString& store_directory,
                         int size)
{
    m_filename = filename;
    if (size > 0)
    {
        m_max_size = size;
    }
    if (store_directory.isEmpty())
    {
        m_store_dir = QApplication::applicationDirPath();
    }
    QDir dir{store_directory};
    if (!dir.exists())
    {
        m_store_dir = QApplication::applicationDirPath();
        dir.setPath(m_store_dir);
    }
    QString app_history_dir = QString::fromUtf8("app_history");
    if (!dir.cd(app_history_dir))
    {
        dir.mkdir(app_history_dir);
    }
    m_store_dir += QString::fromUtf8("/") + app_history_dir;
}

EditHistory::~EditHistory()
{
    this->removeUnused(0);
    QDir dir(m_store_dir);
    dir.removeRecursively();
    dir.setPath(this->getStoreDirectory());
    dir.mkdir(QString::fromUtf8("app_history"));
}

void EditHistory::setFileName(const QString& name)
{
    m_filename = name;
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
        m_store_dir = dir;
        QString app_history_dir = QString::fromUtf8("app_history");
        if (!d.cd(app_history_dir))
        {
            d.mkdir(app_history_dir);
        }
        m_store_dir += QString::fromUtf8("/") + app_history_dir;
        return true;
    }
    return false;
}

QString EditHistory::getStoreDirectory()
{
    return QString(m_store_dir).remove(m_store_dir.length() - 12, 12);
}

bool EditHistory::setMaxStoredCount(int count)
{
    if (count > 0)
    {
        m_max_size = count;
        this->uploadExcessImages();
        return true;
    }
    return false;
}

int EditHistory::getMaxStoredCount()
{
    return m_max_size;
}

void EditHistory::add(const QImage& image)
{
    // If list is still not full.
    if (m_stored_count < m_max_size)
    {
        this->removeUnused(m_pointer + 1);
        m_list.push_back(new QImage(image.copy()));
        m_stored_count = m_list.size();
        ++m_pointer;
        return;
    }

    int dif = m_stored_count - m_max_size;
    if (m_pointer < dif)
    {
        QDir dir(m_store_dir);
        for (int i = m_pointer + 1; i < dif; ++i)
        {
            dir.remove(QString::number(i) +
                       QString::fromUtf8("__") +
                       m_filename);
        }
        this->removeUnused(0);

        for (int i = 0; i <= m_pointer; ++i)
        {
            m_list.push_back(new QImage(QString::number(i) +
                                        QString::fromUtf8("__") +
                                        m_filename));
        }
    }
    else
    {
        this->removeUnused(m_pointer - dif + 1);

        int free_in_list = m_max_size - (m_pointer - dif + 1);
        int to_load = qMin(free_in_list, dif);
        int load_start_index = qMax(0, dif - free_in_list);
        for (int i = load_start_index; i < load_start_index + to_load; ++i)
        {
            m_list.push_back(new QImage(
                m_store_dir + QString::fromUtf8("/") +
                QString::number(i) + QString::fromUtf8("__") + m_filename));
        }
    }
    m_list.push_back(new QImage(image.copy()));
    m_stored_count = m_pointer + 2;
    ++m_pointer;
    this->uploadExcessImages();
}

QImage* EditHistory::back()
{
    if (m_pointer < 1)
    {
        return nullptr;
    }

    --m_pointer;
    QImage* res = this->get();

    return res;
}

QImage* EditHistory::forward()
{
    if (m_pointer == (m_stored_count - 1))
    {
        return nullptr;
    }

    ++m_pointer;
    QImage* res = this->get();

    return res;
}

QImage* EditHistory::get()
{
    if (m_stored_count <= m_max_size)
    {
        return new QImage(m_list[m_pointer]->copy());
    }
    else
    {
        const int dif = m_stored_count - m_max_size;
        if (m_pointer < dif)    // required image is on hard disk.
        {
            return new QImage{m_store_dir + QString::fromUtf8("/") +
                   QString::number(m_pointer) + QString::fromUtf8("__") +
                   m_filename};
        }
        else    // required image is in main memory.
        {
            return new QImage(m_list[m_pointer - dif]->copy());
        }
    }
}

void EditHistory::clean()
{
    m_stored_count = 0;
    m_pointer = -1;
    this->removeUnused(0);

    QDir dir(m_store_dir);
    dir.removeRecursively();

    dir.setPath(this->getStoreDirectory());
    dir.mkdir(QString::fromUtf8("app_history"));
}

void EditHistory::uploadExcessImages()
{
    if (m_list.size() <= m_max_size)
    {
        return;
    }
    int count_to_upload = m_list.size() - m_max_size;
    int name_index = m_stored_count - m_max_size - 1;
    while (count_to_upload != 0)
    {
        if (m_list.front() != nullptr)
        {
            m_list.front()->save(m_store_dir + QString::fromUtf8("/") +
                     QString::number(name_index) +
                     QString::fromUtf8("__") +
                     m_filename, 0, 100);
            delete m_list.front();
        }
        m_list.pop_front();
        --count_to_upload;
        ++name_index;
    }
}

void EditHistory::removeUnused(int loc_pos)
{
    if ((loc_pos < 0) || (loc_pos >= m_list.size()))
    {
        return;
    }
    while (m_list.size() != loc_pos)
    {
        if (m_list.back() != nullptr)
        {
            delete m_list.back();
        }
        m_list.pop_back();
    }
}
