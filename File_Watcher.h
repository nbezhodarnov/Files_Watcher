#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H

#include <QBitArray>
#include <QObject>
#include <QMutex>

class File_Watcher : public QObject {
    Q_OBJECT
public:
    File_Watcher(File_Watcher &) = delete;
    void operator=(const File_Watcher &) = delete;

    static File_Watcher* GetInstance(QObject *parent = nullptr, QString *out = nullptr);

    QStringList get_files_list();
    bool add_file(const QString &file);
    void remove_file(quint64 number);
    quint64 get_size(quint64 number);
    ~File_Watcher();

signals:
    void file_disappeared(const QString &file, QString *out_line);
    void file_appeared(const QString &file, QString *out_line);
    void file_changed(const QString &file, QString *out_line);

public slots:
    void check();

private:
    File_Watcher(QObject *parent = nullptr, QString *out = nullptr);
    QStringList files_list;
    QList<quint64> files_sizes;
    QBitArray file_exists;
    QString *last_line;

    static File_Watcher *pinstance_;
    static QMutex mutex_;
};

#endif //FILE_WATCHER_H
