//
// Created by Елена on 15/01/2019.
//

#ifndef FILEMANAGER_TRIGRAM_PROCESS_H
#define FILEMANAGER_TRIGRAM_PROCESS_H

#include <QThread>
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

class finderTrig: public QMainWindow {
Q_OBJECT

public:
    finderTrig(QString dir);
    ~finderTrig();

public slots:
    void process();

signals:
    void addFileTrigrams(fileTrigram file);
    void finished();

private:
    void startPreprocessing();
    bool check(QString name);
    int makeTrig(char a, char b, char c);
    //void addTrigrams(QString const name, std::unordered_set<int> &set);
    void addTrigrams(fileTrigram &file);

private:
    QString curDir;
};


#endif //FILEMANAGER_TRIGRAM_PROCESS_H
