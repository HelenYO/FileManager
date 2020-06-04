#include <array>
#include "finderOfCopies.h"

struct cancellation_exception : std::exception {
    const char *what() const noexcept override {
        return "Stop thread";
    }
};

void finder::process() {
    scan_directory();
}

finder::finder(QString dir){
    curDir = dir; //NOLINT
}
finder::~finder() = default;


void finder::find_copies(QVector<std::pair<QString, int>> vec,
                              std::vector<std::ifstream> &streams, int degree) {

    try {
        auto cancellation_point = [thread = QThread::currentThread(), this]() {
            if (thread->isInterruptionRequested()) {
                throw cancellation_exception();
            }
        };

        QCryptographicHash sha(QCryptographicHash::Sha3_256);
        std::map<QByteArray, QVector<std::pair<QString, int>>> hashs;
        int gcount = 0;
        for (std::pair<QString, int> file : vec) {
            cancellation_point();
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
        cancellation_point();
        if (gcount == 0) {
            emit addToTree(hashs);
        } else {
            for (auto &ivec : hashs) {
                cancellation_point();
                if (degree < 20) find_copies(ivec.second, streams, degree + 1);
                else find_copies(ivec.second, streams, degree);
            }
        }
    } catch (std::exception &ex) {
        emit error("stop thread");
    }
}

void finder::scan_directory() {
    try {

        auto cancellation_point = [thread = QThread::currentThread(), this]() {
            if (thread->isInterruptionRequested()) {
                throw cancellation_exception();
            }
        };
        //std::clock_t time = std::clock();
        QDirIterator it(curDir, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories); //

        std::map<qint64, QVector<QString>> files;
        while (it.hasNext()) {
            cancellation_point();
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

        cancellation_point();


        long long sumProgressAll = 0;

        for (auto i = files.begin(); i != files.end(); ++i) {
            cancellation_point();
            sumProgressAll += i->second.size() * i->first;
        }

        emit setProgressBar(sumProgressAll);


        QCryptographicHash sha(QCryptographicHash::Sha3_256);
        for (auto i = files.begin(); i != files.end(); ++i) {
            cancellation_point();
            if (i->second.size() != 1) {

                //первая итерация, которая отсечет дофига
                std::map<QByteArray, QVector<std::pair<QString, int>>> hashsFirstIter;
                for (auto name: i->second) {
                    cancellation_point();
                    sha.reset();
                    QFile file(name);
                    std::ifstream fin(name.toStdString(), std::ios::binary);
                    int gcount = 0;
                    if (fin.is_open()) {
                        std::array<char, 4> buffer;
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
                for (auto &vec: hashsFirstIter) {
                    cancellation_point();

                    std::vector<std::ifstream> streams((unsigned long long) (vec.second.size()));
                    for (auto pair: vec.second) {
                        std::ifstream fin(pair.first.toStdString(), std::ios::binary);
                        streams[pair.second] = std::move(fin);
                    }
                    find_copies(vec.second, streams, 0);
                }
                //
            }
        }

        std::cout << "I WAS HERE IN THREAD";

        emit finished();
    } catch (std::exception &ex) {
        emit error("stop thread");
    }
}
