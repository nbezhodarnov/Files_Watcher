#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H

#include <QBitArray>
#include <QObject>

class File_Watcher : public QObject {
    Q_OBJECT
public:
    File_Watcher(QObject *parent = nullptr);
    QStringList get_files_list();
    void add_file(const QString &file);
    void remove_file(quint64 number);
    quint64 get_size(quint64 number);
    ~File_Watcher();

signals:
    void file_disappeared(const QString &file);
    void file_appeared(const QString &file);
    void file_changed(const QString &file);

public slots:
    QBitArray check();

private:
    QStringList files_list;
    QList<quint64> files_sizes;
    class QTimer* timer;
};

#endif //FILE_WATCHER_H
