#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDirIterator>
#include <QCryptographicHash>
#include <fstream>
#include <iostream>


bool wasDuplicate = false;
qint64 sum = 0;

main_window::main_window(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    setWindowTitle(QString("FileManager"));
    ui->pushButton_2->setEnabled(false);

    QCommonStyle style;

    //ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    //ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    connect(ui->actionScan_Directory, &QAction::triggered, this, &main_window::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    //connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);

    connect(ui->pushButton, &QPushButton::clicked, this, &main_window::select_directory);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &main_window::scan_directory);


}

main_window::~main_window() {}

void main_window::select_directory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    setWindowTitle(QString("Directory Content - %1").arg(dir));
    curDir = dir;
    ui->lineEdit->insert(dir);
    ui->pushButton_2->setEnabled(true);

}

void main_window::write_dfs(QDir const &d) {
    QDir dir(d);
    for (QString dirName : dir.entryList(QDir::Dirs)) {
        if (dirName != "." && dirName != "..") {
            QDir curDir = dir;
            curDir.cd(dirName);
            QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
            QFileInfo file_info(dirName);
            item->setText(0, file_info.fileName());
            item->setText(1, QString::number(file_info.size()));
            write_dfs(curDir);
        }
    }
}

void main_window::find_copies(std::map<QByteArray, QVector<std::pair<QString, int>>> &map,
                              std::vector<std::ifstream> &streams, int degree) {

    QCryptographicHash sha(QCryptographicHash::Sha3_256);
    for (auto j = map.begin(); j != map.end(); ++j) {
        if (j->second.size() == 1) {
            streams[j->second[0].second].close();
            break;
        }
        std::map<QByteArray, QVector<std::pair<QString, int>>> hashs;
        //for (QVector<std::pair<QString, int>> names: j->second) {
        int gcount = 0;
        for (std::pair<QString, int> name: j->second) {
            sha.reset();
            std::vector<char> buffer((1<< degree));
            streams[name.second].read(buffer.data(), (1<< degree));
            gcount = static_cast<int>(streams[name.second].gcount());
            sha.addData(buffer.data(), gcount);
            QByteArray res = sha.result();
            std::map<QByteArray, QVector<std::pair<QString, int>>>::iterator cur = hashs.find(res);
            if (cur == hashs.end()) {
                QVector<std::pair<QString, int>> temp;
                temp.push_back(name);
                hashs.insert({res, temp});
            } else {
                cur->second.push_back(name);
            }

            if (gcount ==
                0) {//то есть мы закончили считываие файла (еще есть случай что как раз закончили на размере буфера
                streams[name.second].close();
            }
        }
        if (gcount == 0) {
            for (std::map<QByteArray, QVector<std::pair<QString, int>>>::iterator cur = hashs.begin();
                 cur != hashs.end(); ++cur) {
                if (cur->second.size() != 1) {
                    wasDuplicate = true;
                    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
                    item->setText(0, QString("Found ") + QString::number(cur->second.size()) + QString(" duplicates"));

                    QFileInfo file_info_temp(cur->second.front().first);
                    item->setText(1,
                                  QString::number(file_info_temp.size() * (cur->second.size() - 1)) +
                                  QString(" bytes"));
                    sum += file_info_temp.size() * (cur->second.size() - 1);
                    for (auto child : cur->second) {
                        QTreeWidgetItem *childItem = new QTreeWidgetItem();
                        childItem->setText(0, child.first);
                        item->addChild(childItem);
                    }
                    ui->treeWidget->addTopLevelItem(item);
                }
            }
        } else {
            find_copies(hashs, streams, degree + 1);
        }
        //}
    }
}

void main_window::scan_directory() {

    std::clock_t time = std::clock();
    QDirIterator it(curDir, QDir::Files, QDirIterator::Subdirectories);

    ui->treeWidget->clear();
    QMap<qint64, QVector<QString>> files;
    while (it.hasNext()) {
        QFileInfo file_info(it.next());
        qint64 sizeT = file_info.size();
        if (files.find(sizeT) != files.end()) {
            files.find(sizeT)->push_back(file_info.filePath());
        } else {
            QVector<QString> tempi;
            tempi.push_back(file_info.filePath());
            files.insert(sizeT, tempi);
        }
    }

    QCryptographicHash sha(QCryptographicHash::Sha3_256);
    for (auto i = files.begin(); i != files.end(); ++i) {
        if (i->size() != 1) {

            //первая итерация, которая отсечет дофига
            std::map<QByteArray, QVector<std::pair<QString, int>>> hashsFirstIter;
            for (auto name: (*i)) {
                sha.reset();
                QFile file(name);
                std::ifstream fin(name.toStdString(), std::ios::binary);
                int gcount = 0;
                if (file.open(QIODevice::ReadOnly)) {
                    std::array<char, 1> buffer{};
                    fin.read(buffer.data(), buffer.size());
                    gcount = static_cast<int>(fin.gcount());
                    sha.addData(buffer.data(), gcount);
                }
                QByteArray res = sha.result();
                std::map<QByteArray, QVector<std::pair<QString, int>>>::iterator cur = hashsFirstIter.find(res);
                if (cur == hashsFirstIter.end()) {
                    QVector<std::pair<QString, int>> temp;
                    temp.push_back({name, 0});
                    hashsFirstIter.insert({res, temp});
                } else {
                    cur->second.push_back({name, cur->second.size()});
                }
            }
            //


            //рекурсивный поиск
            for (auto vec: hashsFirstIter) {

                std::vector<std::ifstream> streams(vec.second.size());
                for (auto pair: vec.second) {
                    std::ifstream fin(pair.first.toStdString(), std::ios::binary);
                    streams[pair.second] = std::move(fin);
                }
                find_copies(hashsFirstIter, streams, 0);
            }
            //

        }
    }

    time = std::clock() - time;

    if (!wasDuplicate) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString("Not Found Duplicates!)"));
    } else {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString("In total: ") + QString::number(sum) + QString(" bytes!! (") +
                         QString::number(time / CLOCKS_PER_SEC) + QString(" sec)"));

    }
    sum = 0;
}

void main_window::show_about_dialog() {
    QMessageBox::aboutQt(this);
}
