#ifndef FILEMANAGER_FINDEROFCOPIES_H
#define FILEMANAGER_FINDEROFCOPIES_H


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

#include "mainwindow.h"

class finder : public QObject {
Q_OBJECT

private:
    void scan_directory();

    void find_copies(QVector<std::pair<QString, int>> vec, std::vector<std::ifstream> &streams, int degree);


public:
    explicit finder(QString dir);

    ~finder() override;

public slots:

    void process();

signals:

    void finished();

    void error(QString err);

    void increaseBar(double val);

    void setProgressBar(long long MAXS);

    void addToTree(std::map<QByteArray, QVector<std::pair<QString, int>>> hashs);

private:
    QString curDir;

};

#endif //FILEMANAGER_FINDEROFCOPIES_H