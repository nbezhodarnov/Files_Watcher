#include "File_Watcher.h"

#include <QTextStream>
#include <QTextCodec>
#include <QTimer>
#include <QDebug>
#include <QFileInfo>
#include <iostream>

File_Watcher::File_Watcher(QObject *parent) : QObject(parent) {
    timer = new QTimer(this);
    
    connect(timer, &QTimer::timeout, this, &File_Watcher::check);
    
    timer->start(100);
}

QBitArray File_Watcher::check() {
    QBitArray result(files_list.size());
    if (files_list.size() > 0) {
        for (quint64 i = 0; i < files_list.size(); ++i) {
            if (QFileInfo::exists(files_list[i])) {
                result.setBit(i, true);
                if (files_sizes[i] == 0) {
                    files_sizes[i] = QFileInfo(files_list[i]).size();
                    emit file_appeared(files_list[i]);
                } else if (QFileInfo(files_list[i]).size() != files_sizes[i]) {
                    files_sizes[i] = QFileInfo(files_list[i]).size();
                    emit file_changed(files_list[i]);
                }
            } else {
                if (files_sizes[i] != 0) {
                    files_sizes[i] = 0;
                    emit file_disappeared(files_list[i]);
                }
            }
        }
    }
    return result;
}

QStringList File_Watcher::get_files_list() {
    return files_list;
}

void File_Watcher::add_file(const QString &file) {
    files_list.append(file);
    quint64 size = 0;
    if (QFileInfo(file).exists()) {
        size = QFileInfo(file).size();
    }
    files_sizes.append(size);
    
}

void File_Watcher::remove_file(quint64 number) {
    files_list.removeAt(number);
    files_sizes.removeAt(number);
}

quint64 File_Watcher::get_size(quint64 number) {
    return QFileInfo(files_list[number]).size();
}

File_Watcher::~File_Watcher() {
    delete timer;
}
