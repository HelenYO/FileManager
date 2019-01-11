#include "mainwindow2.h"
#include "ui_MainWindow1.h"
#include "finderOfStrings.h"

#include <QDirIterator>
#include <QFileDialog>
#include <fstream>
#include <iostream>
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <Qthread>
#include <QDirIterator>
#include <QCryptographicHash>
#include <fstream>
#include <iostream>


int BUFFSIZE = 100;

typedef std::pair<QString, std::vector<std::pair<int, int>>> myPair;

subFind::subFind(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow1)
{
    ui->setupUi(this);
    ui->buttonFind->setEnabled(true);
    connect(ui->buttonSelectDir, &QPushButton::clicked, this, &subFind::select_directory);
    connect(ui->buttonFind, &QPushButton::clicked, this, &subFind::start_find);


    qRegisterMetaType<myPair>("myPair");

}

subFind::~subFind() = default;

void subFind::select_directory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    setWindowTitle(QString("Directory Content - %1").arg(dir));
    curDir = dir;

    startPreprocessing();
}

void subFind::start_find() {

    ui->treeWidget->clear();
    std::string sub = ui->lineEditSubString->text().toStdString();

    thread = new QThread;
    auto *worker = new finderSub(curDir, sub, files);
    worker->moveToThread(thread);

    connect(thread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(addToTree(myPair)),
            this, SLOT(addToTreeUI(myPair)));
    thread->start();
}

void subFind::addToTreeUI(std::pair<QString, std::vector<std::pair<int, int>>> add) {

        auto *item = new QTreeWidgetItem(ui->treeWidget);
        //QString temp = QString::mid(contains[i].first);
        QString temp = add.first.mid(curDir.length() + 1,add.first.length() - curDir.length());
        temp +=  "    founded: ";
        for (int j = 0; j < add.second.size(); j++) {
            temp += "\n          ";
            temp += "in ";
            temp += QString::number(add.second[j].first);
            temp += " line ";
            temp += QString::number(add.second[j].second) + " times";
        }

        item->setText(0, temp);
}

void subFind::startPreprocessing() {
    QStringList _filters;
    _filters << "*.txt" << "*.text" << "*.tex" << "*.ttf" << "*.sub" << "*.pwi" << "*.log" << "*.err" << "*.apt";
    QDirIterator it(curDir, _filters, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories); //
    files = *(new std::vector<fileTrigram>());
    while (it.hasNext()) {
        QFileInfo file_info(it.next());
        QString name = file_info.absoluteFilePath();
        files.emplace_back(name);
        addTrigrams(name, files[files.size() - 1].trigrams);
    }
}

void subFind::addTrigrams(QString name, std::set<int> &set) {
    std::ifstream fin(name.toStdString(), std::ios::binary);
    int gcount = -1;
    uint8_t tr1 = 0;
    uint8_t tr2 = 0;
    while(gcount != 0) {
        std::vector<char> buffer(BUFFSIZE);
        fin.read(buffer.data(), BUFFSIZE);
        gcount = static_cast<int>(fin.gcount());
        if(gcount != -1) {
            int ans1 = 0;
            ans1 |= tr1;
            ans1 <<= 8;
            ans1 |= tr2;
            ans1 <<= 8;
            ans1 |= (uint8_t)buffer[0];
            set.insert(ans1);
            if(gcount > 1) {
                int ans2 = 0;
                ans2 |= tr2;
                ans2 <<= 8;
                ans2 |= (uint8_t)buffer[0];
                ans2 <<= 8;
                ans2 |= (uint8_t)buffer[1];
                set.insert(ans2);
            }
        }
        if(gcount == BUFFSIZE) {
            tr1 = (uint8_t)buffer[BUFFSIZE - 2];
            tr2 = (uint8_t)buffer[BUFFSIZE - 1];
        }
        for (int i = 0; i < gcount - 3 + 1; ++i) {
            int ans = 0;
            uint8_t a = (uint8_t)buffer[i];
            ans |= a;
            ans <<= 8;
            a = (uint8_t)buffer[i + 1];
            ans |= a;
            ans <<= 8;
            a = (uint8_t)buffer[i + 2];
            ans |= a;
            set.insert(ans);
        }
    }
}













