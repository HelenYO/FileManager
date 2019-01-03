#include "finderOfCopies.h"

void finder::process() {
    scan_directory();
}

finder::finder(QString dir){
    curDir = dir;
}
finder::~finder() = default;


void finder::find_copies(QVector<std::pair<QString, int>> vec,
                              std::vector<std::ifstream> &streams, int degree) {

    QCryptographicHash sha(QCryptographicHash::Sha3_256);
    std::map<QByteArray, QVector<std::pair<QString, int>>> hashs;
    int gcount = 0;
    for (std::pair<QString, int> file : vec) {
        sha.reset();
        std::vector<char> buffer((1ull << degree));
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
        emit addToTree(hashs);
//        for (auto cur = hashs.begin(); cur != hashs.end(); ++cur) {
//            if (cur->second.size() != 1) {
//                wasDuplicate = true;
//                auto *item = new QTreeWidgetItem(ui->treeWidget);
//                item->setText(0, QString("Found ") + QString::number(cur->second.size()) + QString(" duplicates"));
//
//                QFileInfo file_info_temp(cur->second[0].first);
//                item->setText(1,
//                              QString::number(file_info_temp.size() * (cur->second.size() - 1)) +
//                              QString(" bytes"));
//                sum += file_info_temp.size() * (cur->second.size() - 1);
//                for (auto child : cur->second) {
//                    QTreeWidgetItem *childItem = new QTreeWidgetItem();
//                    childItem->setText(0, child.first.mid(curDir.length() + 1, child.first.length() - curDir.length() - 1));
//                    item->addChild(childItem);
//                }
//                ui->treeWidget->addTopLevelItem(item);
//            }
//            QFileInfo file_info_temp(cur->second[0].first);
//            sumProgress += cur->second.size() * file_info_temp.size();
//            ui->progressBar->setValue((int)(100 * sumProgress / sumProgressAll));
//        }
    } else {
        for (auto ivec : hashs) {
            if (degree < 20) find_copies(ivec.second, streams, degree + 1);
            else find_copies(ivec.second, streams, degree);
        }
    }


}

void finder::scan_directory() {
//    emit increaseBar(0);
    //ui->progressBar->setValue(0);
    std::clock_t time = std::clock();
    QDirIterator it(curDir, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories); //

    //ui->treeWidget->clear();
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


    //

    long long sumProgressAll = 0;

    for(auto i = files.begin(); i != files.end(); ++i) {
        sumProgressAll += i->second.size() * i->first;
    }

    emit setProgressBar(sumProgressAll);

    //

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
                if (fin.is_open()) {
                    std::array<char, 4> buffer{};
                    fin.read(buffer.data(), buffer.size());
                    gcount = static_cast<int>(fin.gcount());
                    sha.addData(buffer.data(), gcount);

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

//    time = std::clock() - time;
//
//    if (!wasDuplicate) {
//        auto *item = new QTreeWidgetItem(ui->treeWidget);
//        item->setText(0, QString("Not Found Duplicates!)"));
//        ui->pushButton_3->setEnabled(false);
//    } else {
//        auto *item = new QTreeWidgetItem(ui->treeWidget);
//        item->setText(0, QString("In total: ") + QString::number(sum) + QString(" bytes!! (") +
//                         QString::number(time / CLOCKS_PER_SEC) + QString(" sec)"));
//        ui->pushButton_3->setEnabled(true);
//
//    }
//    wasDuplicate = false;
//    sum = 0;
//    sumProgressAll = 0;
//    sumProgress = 0;

    std::cout<< "I WAS HERE IN THREAD";

    emit finished();
}
