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


class finderTrig: public QMainWindow {
Q_OBJECT

public:
    finderTrig();
    ~finderTrig();

public slots:
    void process();


};


#endif //FILEMANAGER_TRIGRAM_PROCESS_H
