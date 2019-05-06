#ifndef FILEMANAGER_FINDEROFSTRINGS_H
#define FILEMANAGER_FINDEROFSTRINGS_H


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



class finderSub : public QObject {
    Q_OBJECT

public:
    finderSub(QString dir, std::string strSub, std::vector<fileTrigram> filestemp);

    ~finderSub();

private:

    void scan_directory();

public slots:
    void process();

signals:
    void finished();

    void error();

    void setProgressBar();

    void updateProgressBar();

    void addToTree(std::pair<QString, std::vector<std::pair<int, int>>> add);

private:
    QString curDir;
    std::string sub;
    std::vector<fileTrigram> files;
};

#endif //FILEMANAGER_FINDEROFSTRINGS_H