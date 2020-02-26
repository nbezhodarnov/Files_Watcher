#include "File_Watcher.cpp"

#include <QFileSystemWatcher>
#include <QCoreApplication>
#include <QTextStream>
#include <QTextCodec>
#include <QFileInfo>
#include <QObject>
#include <QDebug>
#include <QFile>
#include <future>
#include <iostream>

//printf("\r%80c\r", ' ');

void file_disappeared_notifier(const QString &file) {
    qDebug() << "File " << file << " has been deleted or renamed or its directory has been changed.\n";
}

void file_appeared_notifier(const QString &file) {
    qDebug() << "File " << file << " has been appeared. It has size " << QFileInfo(file).size() << " bytes.\n";
}

void file_changed_notifier(const QString &file) {
    qDebug() << "File " << file << " has been changed. Now it has size " << QFileInfo(file).size() << " bytes.\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    QTimer exitTimer;
    exitTimer.setInterval(500);

    File_Watcher worker;
    QObject::connect(&worker, &File_Watcher::file_disappeared, &file_disappeared_notifier);
    QObject::connect(&worker, &File_Watcher::file_appeared, &file_appeared_notifier);
    QObject::connect(&worker, &File_Watcher::file_changed, &file_changed_notifier);


    bool exitFlag = false;
    auto f = std::async(std::launch::async, [&exitFlag, &worker]
        {
            //worker.check();
        });


    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    QTextStream inStream(stdin);

    QString input;

    QObject::connect(&exitTimer,
        &QTimer::timeout,
        [&application, &exitFlag, &worker, &inStream, &input]
        {
            std::cout << "Input command: ";
            input = inStream.readLine();

            if (input.toLower() == "add") {
                input = "";
                while (input == "") {
                    std::cout << "Input full path to the file: ";
                    input = inStream.readLine().trimmed().remove('\'');
                }
                if (worker.add_file(input)) {
                    qDebug() << "The file has been successfully added to observation.";
                } else {
                    qDebug() << "The file is already on observation.";
                }
                if (QFileInfo(input).exists()) {
                    qDebug() << "Now this file exists.\n";
                } else {
                    qDebug() << "Now this file doesn't exists.\n";
                }
                if (input.toLower() == "exit") {
                    input = "";
                }
            } else if (input.toLower() == "remove") {
                QStringList files_list = worker.get_files_list();
                if (files_list.size()) {
                    qDebug() << "Choose number of file to remove:";
                    quint64 number;
                    for (int i = 0; i < files_list.size(); ++i) {
                        qDebug() << i + 1 << " - " << files_list[i];
                    }
                    bool is_number = false;
                    while (!is_number) {
                        std::cout << "Input number: ";
                        number = inStream.readLine().toLong(&is_number);
                    }
                    if ((number > files_list.size()) || (number == 0)) {
                        qDebug() << "There is no such file. The command has been canceled.\n";
                    } else {
                        worker.remove_file(number - 1);
                        qDebug() << "The file has been successfully removed from observation.\n";
                    }
                } else {
                    qDebug() << "There are no files. You must add a file to use this command. The command has been canceled.\n";
                }

            } else if (input.toLower() == "size") {
                QStringList files_list = worker.get_files_list();
                if (files_list.size()) {
                    qDebug() << "Choose number of file to see a size:";
                    quint64 number;
                    for (int i = 0; i < files_list.size(); ++i) {
                        qDebug() << i + 1 << " - " << files_list[i];
                    }
                    bool is_number = false;
                    while (!is_number) {
                        std::cout << "Input number: ";
                        number = inStream.readLine().toLong(&is_number);
                    }
                    if ((number > files_list.size()) || (number == 0)) {
                        qDebug() << "There is no such file. The command has been canceled.\n";
                    } else {
                        if (worker.get_size(number - 1) != (quint64)~0) {
                            qDebug() << "Size of this file is " << worker.get_size(number - 1) << " bytes.\n";
                        } else {
                            qDebug() << "I can't open this file. The command has been canceled.\n";
                        }
                    }
                } else {
                    qDebug() << "There are no files. You must add a file to use this command. The command has been canceled.\n";
                }
            } else if ((input.toLower() != "exit") && (input != "")) {
                if (input.toLower() != "help") {
                    qDebug() << "Unknown command: " << input << '\n';
                }
                qDebug() << "Available commands:\n add - add a file to watch\n remove - remove a file from watch\n size - see size of a file\n exit - exit from the program\n help - see a list of commands\n";
            } else if (input.toLower() == "exit") {
                exitFlag = true;
            }

            if (exitFlag)
                application.quit();
        });

    exitTimer.start();

    int res = application.exec();
    f.wait();

    return res;
}
