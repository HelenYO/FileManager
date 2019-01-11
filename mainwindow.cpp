#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "finderOfCopies.h"


#include <QCommonStyle>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <Qthread>
#include <QDirIterator>
#include <QCryptographicHash>
#include <fstream>
#include <iostream>

typedef std::map<QByteArray, QVector<std::pair<QString, int>>>  mapToTree;


main_window::main_window(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    setWindowTitle(QString("FileManager"));
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);
    ui->pushButton_4->setEnabled(false);
    ui->stopButton->setEnabled(false);
    ui->progressBar->setValue(0);
    ui->label->clear();

    QCommonStyle style;

    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));

    connect(ui->actionScan_Directory, &QAction::triggered, this, &main_window::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);

    connect(ui->pushButton, &QPushButton::clicked, this, &main_window::select_directory);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &main_window::start_search);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &main_window::select_useless);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &main_window::delete_useless);
    connect(ui->stopButton, &QPushButton::clicked, this, &main_window::interruption);

    //
    qRegisterMetaType<mapToTree>("mapToTree");

}

void main_window::interruption() {
    thread->requestInterruption();
}

main_window::~main_window() = default;

void main_window::stop() {

}

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
    ui->treeWidget->clear();
}



void main_window::start_search() {
    ui->progressBar->setValue(0);
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);

    ui->treeWidget->clear();
    thread = new QThread;
    auto *worker = new finder(curDir);

    //here is your signals
    //...


    worker->moveToThread(thread);

    connect(thread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(addToTree(mapToTree)),
            this, SLOT(addToTreeUI(mapToTree)));
    connect(worker, SIGNAL(setProgressBar(long long)), this, SLOT(setProgress(long long)));
    connect(worker, SIGNAL(finished()), this, SLOT(doFinishThings()));

    time = std::clock();
    ui->stopButton->setEnabled(true);
    thread->start();


}

void main_window::setProgress(long long MAXS) {
    ui->progressBar->setMaximum((int)std::abs(MAXS));
}

void main_window::doFinishThings() {
    ui->stopButton->setEnabled(false);
    ui->progressBar->maximum();
    time = std::clock() - time;

    if (ui->treeWidget->topLevelItemCount() == 0) {
        auto *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString("Not Found Duplicates!)"));
        ui->pushButton_3->setEnabled(false);
    } else {
//        auto *item = new QTreeWidgetItem(ui->treeWidget);
//        item->setText(0, QString("In total: ") + QString::number(sum) + QString(" bytes!! (") +
//                         QString::number(time / CLOCKS_PER_SEC) + QString(" sec)"));
        ui->pushButton_3->setEnabled(true);

        ui->label->clear();
        ui->label->setText(QString("In total: ") + QString::number(sum) + QString(" bytes!! (") +
                           QString::number(time / CLOCKS_PER_SEC) + QString(" sec)"));

    }
}

void main_window::addToTreeUI(std::map<QByteArray, QVector<std::pair<QString, int>>> hashs) {
    for (auto cur = hashs.begin(); cur != hashs.end(); ++cur) {
        if (cur->second.size() != 1) {
            //wasDuplicate = true;
            auto *item = new QTreeWidgetItem(ui->treeWidget);
            item->setText(0, QString("Found ") + QString::number(cur->second.size()) + QString(" duplicates"));

            QFileInfo file_info_temp(cur->second[0].first);
            item->setText(1,
                          QString::number(file_info_temp.size() * (cur->second.size() - 1)) +
                          QString(" bytes"));
            sum += file_info_temp.size() * (cur->second.size() - 1);
            for (auto child : cur->second) {
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                childItem->setText(0, child.first.mid(curDir.length() + 1, child.first.length() - curDir.length() - 1));
                item->addChild(childItem);
            }
            ui->treeWidget->addTopLevelItem(item);
        }
        QFileInfo file_info_temp(cur->second[0].first);
        sumProgress += cur->second.size() * file_info_temp.size();
        ui->progressBar->setValue(ui->progressBar->value() + (int)sumProgress);
        //ui->progressBar->setValue((int) (100 * sumProgress / sumProgressAll));
    }

}

void main_window::select_useless() {
    auto root = ui->treeWidget->invisibleRootItem();
    for (int i = 0; i < root->childCount(); ++i) {
        auto* top_item = root->child(i);
        for (int j = 1; j < top_item->childCount(); j++) {
            auto child_item = top_item->child(j);
            child_item->setSelected(true);
        }
    }
    ui->pushButton_4->setEnabled(true);
}

void main_window::delete_useless() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Deleting useless",
                                                              "All files will be deleted forever. \nDo you really want to continue?");
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

void main_window::show_about_dialog() {
    QMessageBox::aboutQt(this);
}
