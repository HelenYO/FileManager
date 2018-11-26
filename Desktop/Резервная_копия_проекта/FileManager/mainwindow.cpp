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

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
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

main_window::~main_window()
{}

void main_window::select_directory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    setWindowTitle(QString("Directory Content - %1").arg(dir));
    curDir = dir;
    ui->lineEdit->insert(dir);
    ui->pushButton_2->setEnabled(true);

}

void main_window::write_dfs(QDir const& d) {
    QDir dir(d);
    for (QString dirName : dir.entryList(QDir::Dirs)) {
            if (dirName != "." && dirName != "..") {
                QDir curDir = dir;
                curDir.cd(dirName);
                        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
                        QFileInfo file_info(dirName);
                        item->setText(0, file_info.fileName());
                        item->setText(1, QString::number(file_info.size()));
                write_dfs(curDir);
            }
        }
}

void main_window::scan_directory()
{

    std::clock_t time = std::clock();
    QDirIterator it(curDir,QDir::Files, QDirIterator::Subdirectories);

    ui->treeWidget->clear();
    QMap<qint64, QVector<QString>> files;
        while (it.hasNext()) {
            QFileInfo file_info(it.next());
            if(files.find(file_info.size()) != files.end()) {
                files.find(file_info.size())->push_back(file_info.filePath());
            } else {
                QVector<QString> tempi;
                tempi.push_back(file_info.filePath());
                files.insert(file_info.size(), tempi);
            }
        }

        bool wasDuplicate = false;
        qint64 sum = 0;

        QCryptographicHash sha(QCryptographicHash::Sha3_256);
        for(auto i = files.begin(); i != files.end(); ++i) {

            if (i->size() != 1) {
                QMap<QByteArray, QVector<QString>> hashs;
                for(auto name: (*i)) {
                    sha.reset();
                    QFile file(name);
                    if(file.open(QIODevice::ReadOnly)) {
                        sha.addData(&file);
                        file.close();
                    }
                    QByteArray res = sha.result();
                    QMap<QByteArray, QVector<QString>>::iterator cur = hashs.find(res);
                    if(cur == hashs.end()) {
                        QVector<QString> temp;
                        temp.push_back(name);
                        hashs.insert(res, temp);
                    } else {
                        cur->push_back(name);
                    }
                }

                for(QMap<QByteArray, QVector<QString>>::iterator cur = hashs.begin(); cur != hashs.end(); ++cur) {
                    if (cur->size() != 1) {
                        wasDuplicate = true;
                        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
                        item->setText(0, QString("Found ") + QString::number(cur->size()) + QString(" duplicates"));

                        QFileInfo file_info_temp(cur->front());
                        item->setText(1, QString::number(file_info_temp.size() * (cur->size() - 1)) + QString(" bytes"));
                        sum += file_info_temp.size() * (cur->size() - 1);
                        for (QString child : *cur) {
                           QTreeWidgetItem* childItem = new QTreeWidgetItem();
                           childItem->setText(0, child);
                           item->addChild(childItem);
                        }
                        ui->treeWidget->addTopLevelItem(item);
                    }
                }

            }
        }

        time = std::clock() - time;

        if(!wasDuplicate) {
            QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
            item->setText(0, QString("Not Found Duplicates!)"));
        } else {
            QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
            item->setText(0, QString("In total: ") + QString::number(sum/1000000) + QString( " Mb!! (") + QString::number(time / CLOCKS_PER_SEC) + QString( " sec)") );
        }
}

void main_window::show_about_dialog()
{
    QMessageBox::aboutQt(this);
}
