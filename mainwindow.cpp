#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QCryptographicHash>
#include <fstream>
#include <iostream>


bool wasDuplicate = false;
qint64 sum = 0;
qint64 sumProgress = 0;
qint64 sumProgressAll = 0;

main_window::main_window(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    setWindowTitle(QString("FileManager"));
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_4->setEnabled(false);
    ui->progressBar->setValue(0);

    QCommonStyle style;

    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));

    connect(ui->actionScan_Directory, &QAction::triggered, this, &main_window::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);

    connect(ui->pushButton, &QPushButton::clicked, this, &main_window::select_directory);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &main_window::scan_directory);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &main_window::select_useless);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &main_window::delete_useless);

}

main_window::~main_window() = default;

void main_window::select_directory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    setWindowTitle(QString("Directory Content - %1").arg(dir));
    curDir = dir;
    ui->lineEdit->clear();
    ui->lineEdit->insert(dir);
    ui->pushButton_2->setEnabled(true);
    ui->progressBar->setValue(0);

}

void main_window::select_useless(){
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        auto *top_item = ui->treeWidget->topLevelItem(i);
        for (int j = 1; j < top_item->childCount(); j++) {
            auto *child_item = top_item->child(j);
            child_item->setSelected(true);
        }
    }
    ui->pushButton_4->setEnabled(true);
}

void main_window::delete_useless(){
    QMessageBox::StandardButton reply = QMessageBox::question(this, "deleting useless",
                                                              "All files will be deleted forever. \nDo you really want to continiue?");
    if (reply == QMessageBox::Yes) {
        QVector<QString> paths;
        for (auto u : ui->treeWidget->selectedItems()) {
            paths.push_back(curDir + '/' + u->text(0));
            u->~QTreeWidgetItem();
        }
        for (int i = 0; i < paths.size(); i++) {
            QFile(paths[i]).remove();
        }
    }
}

void main_window::find_copies(QVector<std::pair<QString, int>> vec,
                              std::vector<std::ifstream> &streams, int degree) {

    QCryptographicHash sha(QCryptographicHash::Sha3_256);
    std::map<QByteArray, QVector<std::pair<QString, int>>> hashs;
    int gcount = 0;
    for (std::pair<QString, int> file : vec) {


        sha.reset();
        std::vector<char> buffer((unsigned long long)(1 << degree));

        streams[file.second].read(buffer.data(), (1 << degree));
        gcount = static_cast<int>(streams[file.second].gcount());
        sha.addData(buffer.data(), gcount);
        QByteArray res = sha.result();
        std::map<QByteArray, QVector<std::pair<QString, int>>>::iterator cur = hashs.find(res);
        if (cur == hashs.end()) {
            QVector<std::pair<QString, int>> temp;
            temp.push_back(file);
            hashs.insert({res, temp});
        } else {
            cur->second.push_back(file);
        }

        if (gcount == 0) {
            streams[file.second].close();
        }

    }
    if (gcount == 0) {
        for (auto cur = hashs.begin(); cur != hashs.end(); ++cur) {
            if (cur->second.size() != 1) {
                wasDuplicate = true;
                auto *item = new QTreeWidgetItem(ui->treeWidget);
                item->setText(0, QString("Found ") + QString::number(cur->second.size()) + QString(" duplicates"));

                QFileInfo file_info_temp(cur->second[0].first);
                item->setText(1,
                              QString::number(file_info_temp.size() * (cur->second.size() - 1)) +
                              QString(" bytes"));
                sum += file_info_temp.size() * (cur->second.size() - 1);
                for (auto child : cur->second) {
                    QTreeWidgetItem *childItem = new QTreeWidgetItem();
                    //childItem->setText(0, child.first);
                    childItem->setText(0, child.first.mid(curDir.length() + 1, child.first.length() - curDir.length() - 1));
                    item->addChild(childItem);
                }
                ui->treeWidget->addTopLevelItem(item);
            }
            QFileInfo file_info_temp(cur->second[0].first);
            sumProgress += cur->second.size() * file_info_temp.size();
            ui->progressBar->setValue(100 * sumProgress / sumProgressAll);
        }
    } else {
        for (auto ivec : hashs)
            find_copies(ivec.second, streams, degree + 1);
    }
}

void main_window::scan_directory() {


    ui->progressBar->setValue(0);
    std::clock_t time = std::clock();
    QDirIterator it(curDir, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories); //

    ui->treeWidget->clear();
    std::map<qint64, QVector<QString>> files;
    while (it.hasNext()) {
        QFileInfo file_info(it.next());
        qint64 sizeT = file_info.size();
        if (files.find(sizeT) != files.end()) {
            files.find(sizeT)->second.push_back(file_info.filePath());
        } else {
            QVector<QString> tempi;
            tempi.push_back(file_info.filePath());
            files.insert({sizeT, tempi});
        }
    }

    for(auto i = files.begin(); i != files.end(); ++i) {
        sumProgressAll += i->second.size() * i->first;
    }

    QCryptographicHash sha(QCryptographicHash::Sha3_256);
    for (auto i = files.begin(); i != files.end(); ++i) {
        if (i->second.size() != 1) {

            //первая итерация, которая отсечет дофига
            std::map<QByteArray, QVector<std::pair<QString, int>>> hashsFirstIter;
            for (auto name: i->second) {
                sha.reset();
                QFile file(name);
                std::ifstream fin(name.toStdString(), std::ios::binary);
                int gcount = 0;
                //if (file.open(QIODevice::ReadOnly)) {
                if (fin.is_open()) {
                    std::array<char, 4> buffer{};
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
                    int temp = cur->second.size();
                    cur->second.push_back({name, temp});
                }
            }
            //

            //рекурсивный поиск
            for (auto vec: hashsFirstIter) {

                std::vector<std::ifstream> streams((unsigned long long)(vec.second.size()));
                for (auto pair: vec.second) {
                    std::ifstream fin(pair.first.toStdString(), std::ios::binary);
                    streams[pair.second] = std::move(fin);
                }
                find_copies(vec.second, streams, 0);
            }
            //

        }
    }

    time = std::clock() - time;

    if (!wasDuplicate) {
        auto *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString("Not Found Duplicates!)"));
    } else {
        auto *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString("In total: ") + QString::number(sum) + QString(" bytes!! (") +
                         QString::number(time / CLOCKS_PER_SEC) + QString(" sec)"));

    }
    sum = 0;
    sumProgressAll = 0;
    sumProgress = 0;
    ui->progressBar->setValue(100);
}

void main_window::show_about_dialog() {
    QMessageBox::aboutQt(this);
}
