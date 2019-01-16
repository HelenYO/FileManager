//
// Created by Елена on 15/01/2019.
//

#ifndef FILEMANAGER_TRIGRAM_PROCESS_H
#define FILEMANAGER_TRIGRAM_PROCESS_H

#include <QThread>
#include <QObject>
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QCryptographicHash>
#include <fstream>
#include <iostream>
#include <QMainWindow>
#include <memory>
#include <QDir>
#include <QDesktopWidget>
#include <QTreeWidget>

#include "fileTrigram.h"

class finderTrig : public QObject {
Q_OBJECT

public:
    finderTrig(QString dir);
    ~finderTrig();

public slots:
    void process();

signals:
    void addFileTrigrams(fileTrigram file);
    void finished();
    void addToWatcher(QString name);
    void increaseBarTrig();
    void setBar(int max);
    void error();

public:
    void startPreprocessing();
    bool check(QString name);
    static int makeTrig(char a, char b, char c);
    static void addTrigrams(fileTrigram &file);

private:
    QString curDir;
    std::vector<QString> files;
};


#endif //FILEMANAGER_TRIGRAM_PROCESS_H
