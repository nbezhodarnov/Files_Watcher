#include "File_Watcher.cpp"

#include <QFileSystemWatcher>
#include <QCoreApplication>
#include <QSignalMapper>
#include <QTextStream>
#include <QTextCodec>
#include <QFileInfo>
#include <QObject>
#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <future>
#include <iostream>

#define ESC "\033"
#define gotoxy(x,y)		printf(ESC "[%d;%dH", y, x);


//printf("\r%80c\r", ' ');
// \033[15C

void file_disappeared_notifier(const QString &file, QString *out_line, QString *in_line) {
    QTextStream(stdout) << "\r%80c\r" << "File " << file << " has been deleted or renamed or its directory has been changed.\n" << *out_line << flush;
}

void file_appeared_notifier(const QString &file, QString *out_line, QString *in_line) {
    QTextStream(stdout) << "\r%80c\r" << "File " << file << " has been appeared. It has size " << QFileInfo(file).size() << " bytes.\n" << *out_line << flush;
}

void file_changed_notifier(const QString &file, QString *out_line, QString *in_line) {
    QTextStream(stdout) << "\r%80c\r" << "File " << file << " has been changed. Now it has size " << QFileInfo(file).size() << " bytes.\n" << *out_line << flush;
}

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    QTimer exitTimer;
    exitTimer.setInterval(500);

    QString input, output;
    File_Watcher worker(nullptr, &output, &input);

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

    QTextStream inStream(stdin), outStream(stdout);

    std::thread terminal(
        [&application, &worker, &inStream, &outStream, &input, &output]
        {
        forever {
            outStream << "Input command: " << flush;
            output = "Input command: ";
            /*input.clear();
            input.replace(0, 1, '\0');
            int j = 0;
            QChar symbol;
            while(input.at(j) != '\n') {
                inStream >> symbol;
                input.append(symbol);
                qDebug() << input;
                j++;
            }
            input.replace(j, 1, '\0');
            input = input.trimmed().remove('\u0000');*/
            input = inStream.readLine().trimmed();

            if (input.toLower() == "add") {
                input = "";
                output = "Input full path to the file: ";
                while (input == "") {
                    outStream << "Input full path to the file: " << flush;
                    input = inStream.readLine().trimmed().remove('\'');
                }
                QFileInfo folder_check(input);
                if ((folder_check.exists()) && (folder_check.isDir())) {
                    output = "It's a folder. Do you want to add all files from it (y/n)? ";
                    outStream << "It's a folder. Do you want to add all files from it (y/n)? " << flush;
                    QString confirmator = inStream.readLine().trimmed();
                    if ((confirmator.toLower() == "yes") || (confirmator.toLower() == "y")) {
                        QStringList files = QDir(input).entryList(QDir::Files, QDir::Name);
                        for (quint64 i = 0; i < files.size(); i++) {
                            if (worker.add_file(files[i])) {
                                outStream << "File " << files[i] << " has been successfully added to observation.\n" << flush;
                            }
                        }
                    } else {
                        outStream << "The command has been canceled.\n" << flush;
                    }
                } else {
                    if (worker.add_file(input)) {
                        outStream << "The file has been successfully added to observation.\n" << flush;
                    } else {
                        outStream << "The file is already on observation.\n" << flush;
                    }
                    if (QFileInfo(input).exists()) {
                        outStream << "Now this file exists.\n" << flush;
                    } else {
                        outStream << "Now this file doesn't exists.\n" << flush;
                    }
                    if (input.toLower() == "exit") {
                        input = "";
                    }
                }
            } else if (input.toLower() == "remove") {
                QStringList files_list = worker.get_files_list();
                if (files_list.size()) {
                    outStream << "Choose number of file to remove:\n" << flush;
                    quint64 number;
                    for (quint64 i = 0; i < files_list.size(); ++i) {
                        outStream << i + 1 << " - " << files_list[i] << '\n' << flush;
                    }
                    bool is_number = false;
                    output = "Input number: ";
                    while (!is_number) {
                        outStream << "Input number: " << flush;
                        number = inStream.readLine().toLong(&is_number);
                    }
                    if ((number > files_list.size()) || (number == 0)) {
                        outStream << "There is no such file. The command has been canceled.\n" << flush;
                    } else {
                        worker.remove_file(number - 1);
                        outStream << "The file has been successfully removed from observation.\n" << flush;
                    }
                } else {
                    outStream << "There are no files. You must add a file to use this command. The command has been canceled.\n" << flush;
                }

            } else if (input.toLower() == "size") {
                QStringList files_list = worker.get_files_list();
                if (files_list.size()) {
                    outStream << "Choose number of file to see a size:\n" << flush;
                    quint64 number;
                    for (quint64 i = 0; i < files_list.size(); ++i) {
                        outStream << i + 1 << " - " << files_list[i] << '\n' << flush;
                    }
                    bool is_number = false;
                    output = "Input number: ";
                    while (!is_number) {
                        outStream << "Input number: " << flush;
                        number = inStream.readLine().toLong(&is_number);
                    }
                    if ((number > files_list.size()) || (number == 0)) {
                        outStream << "There is no such file. The command has been canceled.\n" << flush;
                    } else {
                        if (worker.get_size(number - 1) != (quint64)~0) {
                            outStream << "Size of this file is " << worker.get_size(number - 1) << " bytes.\n" << flush;
                        } else {
                            outStream << "I can't open this file. The command has been canceled.\n" << flush;
                        }
                    }
                } else {
                    outStream << "There are no files. You must add a file to use this command. The command has been canceled.\n" << flush;
                }
            } else if ((input.toLower() != "exit") && (input != "")) {
                if (input.toLower() != "help") {
                    outStream << "Unknown command: " << input << '\n' << flush;
                }
                outStream << "Available commands:\n add - add a file to watch\n remove - remove a file from watch\n size - see size of a file\n exit - exit from the program\n help - see a list of commands\n" << flush;
            } else if (input.toLower() == "exit") {
                break;
            }
           }
        application.quit();
        });
    terminal.detach();

    exitTimer.start();

    f.wait();

    return application.exec();
}
