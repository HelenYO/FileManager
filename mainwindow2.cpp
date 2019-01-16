#include "mainwindow2.h"
#include "ui_mainwindow1.h"
#include "finderOfStrings.h"
#include "trigram_process.h"

#include <QDirIterator>
#include <QFileDialog>
#include <fstream>
#include <iostream>
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QDirIterator>
#include <QCryptographicHash>
#include <fstream>
#include <iostream>

typedef std::pair<QString, std::vector<std::pair<int, int>>> myPair;

subFind::subFind(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::mainwindow1) {
    ui->setupUi(this);
    ui->buttonFind->setEnabled(true);
    ui->buttonStop->setEnabled(false);
    ui->progressBar->setValue(0);
    ui->progressBarPreprocess->setValue(0);
    ui->label->clear();
    connect(ui->buttonSelectDir, &QPushButton::clicked, this, &subFind::select_directory);
    connect(ui->buttonFind, &QPushButton::clicked, this, &subFind::start_find);
    connect(ui->buttonStop, &QPushButton::clicked, this, &subFind::interruption);
    connect(ui->pushButtonStopPreprocess, &QPushButton::clicked, this, &subFind::interruptionTrig);

    qRegisterMetaType<myPair>("myPair");
    qRegisterMetaType<fileTrigram>("fileTrigram");
    //qRegisterMetaType<fileTrigram>("fileTrigram");
    fsWatcher = new QFileSystemWatcher(this);
    connect(fsWatcher, SIGNAL(fileChanged(QString)), this, SLOT(changed(QString)));

}

subFind::~subFind() {
    interruption();
    interruptionTrig();
}

void subFind::change(QString path) {
    ui->label->clear();
    ui->label->setText(path + " was changed");
    int index = 0;
    for (int i = 0; i < files.size(); i++) {
        if (files[i].file == path) {
            index = i;
            break;
        }
    }
    files[index].trigrams.clear();
    finderTrig::addTrigrams(files[index]);
}

void subFind::changed(QString path) {
    if(thread && thread->isRunning()){
        toChange.push_back(path);
    } else {
        change(path);
    }
}

void subFind::select_directory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    setWindowTitle(QString("Directory Content - %1").arg(dir));
    curDir = dir;

    ui->label->clear();
    startPreprocess();
}

void subFind::startPreprocess() {
    files.clear();
    ui->buttonFind->setEnabled(false);
    ui->label->clear();
    ui->label->setText("Preprocessing");
    ui->pushButtonStopPreprocess->setEnabled(true);


//    int sum = 0;
//    QDirIterator git(curDir, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories); //
//    while (git.hasNext()) {
//        ++sum;
//    }
//    int sum = 8;

//    ui->progressBarPreprocess->setMaximum(sum);
    ui->progressBarPreprocess->setValue(0);

    threadTrig = new QThread;
    auto *worker = new finderTrig(curDir);
    worker->moveToThread(threadTrig);

    connect(threadTrig, SIGNAL(started()), worker, SLOT(process()));
    connect(worker, SIGNAL(finished()), threadTrig, SLOT(quit()));
    connect(worker, SIGNAL(addFileTrigrams(fileTrigram)), this, SLOT(addFileTrigramsToFiles(fileTrigram)));
    connect(worker, SIGNAL(finished()), this, SLOT(finishThings()));
    connect(worker, SIGNAL(addToWatcher(QString)), this, SLOT(addToFSWatcher(QString)));
    connect(worker, SIGNAL(increaseBarTrig()), this, SLOT(increaseBar()));
    connect(worker, SIGNAL(setBar(int)), this, SLOT(set_max_index_bar(int)));

    threadTrig->start();
}

void subFind::set_max_index_bar(int max) {
    ui->progressBarPreprocess->setMaximum(max);
}

void subFind::addToFSWatcher(QString name) {
    fsWatcher->addPath(name);
}

void subFind::finishThings() {
    ui->buttonFind->setEnabled(true);
    ui->label->clear();
    ui->label->setText("Preprocessing is finished");
    ui->pushButtonStopPreprocess->setEnabled(false);
}

void subFind::addFileTrigramsToFiles(fileTrigram add) {
    files.push_back(add);
}

void subFind::interruption() {
    if(thread) {
        thread->requestInterruption();
        ui->label->setText("Stopped");
        ui->buttonStop->setEnabled(false);
    }
}

void subFind::interruptionTrig() {
    if(threadTrig) {
        threadTrig->requestInterruption();
        ui->label->setText("Stopped");
        ui->pushButtonStopPreprocess->setEnabled(false);
    }
}

void subFind::interruptionStart() {
    if(thread) {
        thread->requestInterruption();
    }
}

void subFind::start_find() {

    interruptionStart();
    ui->treeWidget->clear();
    ui->progressBar->setValue(0);
    ui->buttonStop->setEnabled(true);
    std::string sub = ui->lineEditSubString->text().toStdString();

    ui->progressBar->setMaximum(files.size());
    thread = new QThread;
    auto *worker = new finderSub(curDir, sub, files);
    worker->moveToThread(thread);

    connect(thread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(addToTree(myPair)),
            this, SLOT(addToTreeUI(myPair)));
    connect(worker, SIGNAL(finished()), this, SLOT(doFinishThings()));
    connect(worker, SIGNAL(updateProgressBar()), this, SLOT(updBar()));
    time = std::clock();
    thread->start();
}

void subFind::updBar() {
    ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void subFind::increaseBar() {
    ui->progressBarPreprocess->setValue(ui->progressBarPreprocess->value() + 1);
}

void subFind::doFinishThings() {
    time = std::clock() - time;
    if (ui->treeWidget->topLevelItemCount() == 0) {
        auto *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString("Not Found Substring"));
    }
    ui->label->clear();
    ui->label->setText(QString::number(time / CLOCKS_PER_SEC) + QString(" sec)"));
    ui->buttonStop->setEnabled(false);
    for(int i = static_cast<int>(toChange.size() - 1); i >= 0; i--) {
        change(toChange[i]);
    }
    toChange.clear();
}

void subFind::addToTreeUI(std::pair<QString, std::vector<std::pair<int, int>>> add) {

    auto *item = new QTreeWidgetItem(ui->treeWidget);
    QString temp = add.first.mid(curDir.length() + 1, add.first.length() - curDir.length());
    temp += "    founded: ";
    item->setText(0, temp);
    for (int j = 0; j < add.second.size(); j++) {
        auto *itemchild = new QTreeWidgetItem(item);
        QString tempi = "";
        tempi += "in ";
        tempi += QString::number(add.second[j].first);
        tempi += " line ";
        tempi += QString::number(add.second[j].second) + " times";
        itemchild->setText(0, tempi);
    }
}