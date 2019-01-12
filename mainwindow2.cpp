#include "mainwindow2.h"
#include "ui_mainwindow1.h"
#include "finderOfStrings.h"

#include <QDirIterator>
#include <QFileDialog>
#include <fstream>
#include <iostream>
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
//#include <QThread>
#include <QDirIterator>
#include <QCryptographicHash>
#include <fstream>
#include <iostream>


unsigned long long BUFFSIZE = 100;

typedef std::pair<QString, std::vector<std::pair<int, int>>> myPair;

subFind::subFind(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::mainwindow1) {
    ui->setupUi(this);
    ui->buttonFind->setEnabled(true);
    ui->buttonStop->setEnabled(false);
    ui->progressBar->setValue(0);
    ui->label->clear();
    connect(ui->buttonSelectDir, &QPushButton::clicked, this, &subFind::select_directory);
    connect(ui->buttonFind, &QPushButton::clicked, this, &subFind::start_find);
    connect(ui->buttonStop, &QPushButton::clicked, this, &subFind::interruption);


    qRegisterMetaType<myPair>("myPair");
    fsWatcher = new QFileSystemWatcher(this);
    connect(fsWatcher, SIGNAL(fileChanged(QString)), this, SLOT(changed(QString)));

}

subFind::~subFind() = default;

void subFind::changed(QString path) {
    ui->label->clear();
    ui->label->setText(path + " was changed");
    int index = 0;
    for (int i = 0; i < files.size(); i++) {
        if (files[i].file == path) {
            index = i;
            break;
        }
    }
    files[index].trigrams = *new std::set<int>();
    addTrigrams(path, files[index].trigrams);
}

void subFind::select_directory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    setWindowTitle(QString("Directory Content - %1").arg(dir));
    curDir = dir;

    startPreprocessing();
}

void subFind::interruption() {
    if(thread) {
        thread->requestInterruption();
        ui->label->setText("Stopped");
        ui->buttonStop->setEnabled(false);
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

void subFind::doFinishThings() {
    ui->progressBar->maximum();
    time = std::clock() - time;
    if (ui->treeWidget->topLevelItemCount() == 0) {
        auto *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString("Not Found Substring"));
    }
    ui->label->clear();
    ui->label->setText(QString::number(time / CLOCKS_PER_SEC) + QString(" sec)"));
    ui->buttonStop->setEnabled(false);
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

void subFind::startPreprocessing() {
    QDirIterator it(curDir, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories); //
    files = *(new std::vector<fileTrigram>());
    while (it.hasNext()) {
        QFileInfo file_info(it.next());
        QString name = file_info.absoluteFilePath();
        if(check(name) ) {
            files.emplace_back(name);
            addTrigrams(name, files[files.size() - 1].trigrams);
            fsWatcher->addPath(name);
        }
    }
}

bool subFind::check(QString name) {
    std::ifstream fin(name.toStdString(), std::ios::binary);
    QString sub = name.mid(name.length() - 3, 3);
    //std::cout << sub.toStdString();
    if ((sub == "txt") || (sub == "tex") || (sub == "log")) {
        return true;
    }
//    if ((sub == "mp3") || (sub == "jpg") || (sub == "zip") || (sub == "rar") || (sub == ".7z") || (sub == "dmg") || (sub == "jar") || (sub == "png")) {
//        return false;
//    }
    std::vector<char> buffer(BUFFSIZE * 100);
    fin.read(buffer.data(), (int) BUFFSIZE * 100);
    for (int i = 0 ; i < fin.gcount(); i++) {
        if (buffer[i] !=  '\0') {

        } else {
            return false;
        }
    }
    return true;
}

void subFind::addTrigrams(QString name, std::set<int> &set) {
    std::ifstream fin(name.toStdString(), std::ios::binary);
    int gcount = -1;
    uint8_t tr1 = 0;
    uint8_t tr2 = 0;
    while (gcount != 0) {
        std::vector<char> buffer(BUFFSIZE);
        fin.read(buffer.data(), (int) BUFFSIZE);
        gcount = static_cast<int>(fin.gcount());
        if (gcount != -1) {
            int ans1 = 0;
            ans1 |= tr1;
            ans1 <<= 8;
            ans1 |= tr2;
            ans1 <<= 8;
            ans1 |= (uint8_t) buffer[0];
            set.insert(ans1);
            if (gcount > 1) {
                int ans2 = 0;
                ans2 |= tr2;
                ans2 <<= 8;
                ans2 |= (uint8_t) buffer[0];
                ans2 <<= 8;
                ans2 |= (uint8_t) buffer[1];
                set.insert(ans2);
            }
        }
        if (gcount == BUFFSIZE) {
            tr1 = (uint8_t) buffer[BUFFSIZE - 2];
            tr2 = (uint8_t) buffer[BUFFSIZE - 1];
        }
        for (int i = 0; i < gcount - 3 + 1; ++i) {
            int ans = 0;
            uint8_t a = (uint8_t) buffer[i];
            ans |= a;
            ans <<= 8;
            a = (uint8_t) buffer[i + 1];
            ans |= a;
            ans <<= 8;
            a = (uint8_t) buffer[i + 2];
            ans |= a;
            set.insert(ans);
        }
    }
}













