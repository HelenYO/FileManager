#include <unordered_set>
#include "trigram_process.h"

struct cancellation_exception : std::exception {
    const char *what() const noexcept override {
        return "Stop thread";
    }
};

unsigned long long BUFFSIZE = 100;

finderTrig::finderTrig(QString dir) : curDir(std::move(dir)) {}

finderTrig::~finderTrig() = default;

void finderTrig::process() {
    startPreprocessing();
    emit finished();
}

void finderTrig::startPreprocessing() {
    try {

        auto cancellation_point = [thread = QThread::currentThread(), this]() {
            if (thread->isInterruptionRequested()) {
                throw cancellation_exception();
            }
        };

        QDirIterator it(curDir, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories); //
        files.clear();
        while (it.hasNext()) {
            cancellation_point();
            QFileInfo file_info(it.next());
            QString name = file_info.absoluteFilePath();
            if (check(name)) {
                files.push_back(name);
            }
        }
        emit setBar(static_cast<int>(files.size()));
        for (const auto &i : files) {
            cancellation_point();
            fileTrigram file(i);
            addTrigrams(file);
            emit addToWatcher(i);
            emit addFileTrigrams(file);
            emit increaseBarTrig();
        }
    } catch (std::exception &ex) {
        emit error();
    }
}

bool finderTrig::check(QString name) {
    std::ifstream fin(name.toStdString(), std::ios::binary);
//    QString sub = name.mid(name.length() - 3, 3);
//    if ((sub == "txt") || (sub == "tex") || (sub == "log")) {
//        return true;
//    }
//    if ((sub == "mp3") || (sub == "jpg") || (sub == "zip") || (sub == "rar")
//        || (sub == ".7z") || (sub == "dmg") || (sub == "jar") || (sub == "png")) {
//        return false;
//    }
    std::vector<char> buffer(BUFFSIZE * 100);
    fin.read(buffer.data(), (int) BUFFSIZE * 100);
    for (int i = 0; i < fin.gcount(); i++) {
        if (buffer[i] != '\0') {

        } else {
            return false;
        }
    }
    return true;
}

int finderTrig::makeTrig(char a, char b, char c) {
    int ans = 0;
    ans |= (uint8_t) a;
    ans <<= 8;
    ans |= (uint8_t) b;
    ans <<= 8;
    ans |= (uint8_t) c;
    return ans;
}

void finderTrig::addTrigrams(fileTrigram &file) {

    std::ifstream fin(file.file.toStdString(), std::ios::binary);
    int gcount = -1;
    char tr1 = '\0';
    char tr2 = '\0';
    while (gcount != 0) {
        std::vector<char> buffer(BUFFSIZE);
        fin.read(buffer.data(), (int) BUFFSIZE);
        gcount = static_cast<int>(fin.gcount());
        if (gcount != -1) {
            int ans1 = makeTrig(tr1, tr2, buffer[0]);

            file.trigrams.insert(ans1);
            if (gcount > 1) {
                int ans2 = makeTrig(tr2, buffer[0], buffer[1]);
                file.trigrams.insert(ans2);
            }
        }
        if (gcount == BUFFSIZE) {
            tr1 = buffer[BUFFSIZE - 2];
            tr2 = buffer[BUFFSIZE - 1];
        }
        for (int i = 0; i < gcount - 3 + 1; ++i) {
            int ans = makeTrig(buffer[i], buffer[i + 1], buffer[i + 2]);
            file.trigrams.insert(ans);
        }
    }
}