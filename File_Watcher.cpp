#include "File_Watcher.h"

#include <QTextStream>
#include <QTextCodec>
#include <QTimer>
#include <QDebug>
#include <QFileInfo>
#include <iostream>
#include <thread>

File_Watcher::File_Watcher(QObject *parent) : QObject(parent) {
    timer = new QTimer(this);

    //connect(timer, &QTimer::timeout, this, &File_Watcher::check);
    std::thread check_files(&File_Watcher::check, this);
    check_files.detach();

    timer->start(100);
}

void File_Watcher::check() {
    forever {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (files_list.size() > 0) {
        for (quint64 i = 0; i < files_list.size(); ++i) {
            if (QFileInfo::exists(files_list[i])) {
                if (!file_exists.at(i)) {
                    file_exists.setBit(i, true);
                    files_sizes[i] = QFileInfo(files_list[i]).size();
                    emit file_appeared(files_list[i]);
                } else if (QFileInfo(files_list[i]).size() != files_sizes[i]) {
                    files_sizes[i] = QFileInfo(files_list[i]).size();
                    emit file_changed(files_list[i]);
                }
            } else {
                if (file_exists.at(i)) {
                    file_exists.setBit(i, false);
                    files_sizes[i] = 0;
                    emit file_disappeared(files_list[i]);
                }
            }
        }
    }
    }
}

QStringList File_Watcher::get_files_list() {
    return files_list;
}

bool File_Watcher::add_file(const QString &file) {
    if (std::find(files_list.begin(), files_list.end(), file) != files_list.end()) {
        return false;
    }
    files_list.append(file);
    file_exists.resize(file_exists.size() + 1);
    quint64 size = 0;
    if (QFileInfo(file).exists()) {
        size = QFileInfo(file).size();
        file_exists.setBit(file_exists.size() - 1, true);
    }
    files_sizes.append(size);
    return true;
}

void File_Watcher::remove_file(quint64 number) {
    files_list.removeAt(number);
    files_sizes.removeAt(number);
    QBitArray temp(file_exists.size() - 1);
    for (quint64 i = 0; i < number; i++) {
        temp.setBit(i, file_exists.at(i));
    }
    for (quint64 i = number + 1; i < file_exists.size(); i++) {
        temp.setBit(i - 1, file_exists.at(i));
    }
    file_exists = temp;
}

quint64 File_Watcher::get_size(quint64 number) {
    if (!file_exists.at(number)) {
        return ~0;
    }
    return QFileInfo(files_list[number]).size();
}

File_Watcher::~File_Watcher() {
    delete timer;
}
