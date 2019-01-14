//
// Created by Елена on 15/01/2019.
//

#include <unordered_set>
#include "trigram_process.h"


unsigned long long BUFFSIZE = 100;

finderTrig::finderTrig(QString dir) : curDir(dir){}

finderTrig::~finderTrig() {}

void finderTrig::process() {
    startPreprocessing();
    emit finished();
}

void finderTrig::startPreprocessing() {
    QDirIterator it(curDir, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories); //
//    files.clear();
    while (it.hasNext()) {
        QFileInfo file_info(it.next());
        QString name = file_info.absoluteFilePath();
        if(check(name) ) {
            fileTrigram file(name);
            //files.emplace_back(name);
            //addTrigrams(name, files[files.size() - 1].trigrams);
            addTrigrams(file);
            //fsWatcher->addPath(name);//todo: fsWatcher
            emit addFileTrigrams(file);
        }
    }
}

bool finderTrig::check(QString name) {
    std::ifstream fin(name.toStdString(), std::ios::binary);
//    QString sub = name.mid(name.length() - 3, 3);
//    if ((sub == "txt") || (sub == "tex") || (sub == "log")) {
//        return true;
//    }
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


int finderTrig::makeTrig(char a, char b, char c) {
    int ans = 0;
    ans |= (uint8_t)a;
    ans <<= 8;
    ans |= (uint8_t)b;
    ans <<= 8;
    ans |= (uint8_t)c;
    return ans;
}

//void finderTrig::addTrigrams(QString const name, std::unordered_set<int> &set) {
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
            //set.insert(ans1);
            file.trigrams.insert(ans1);
            if (gcount > 1) {
                int ans2 = makeTrig(tr2, buffer[0], buffer[1]);
                //set.insert(ans2);
                file.trigrams.insert(ans2);
            }
        }
        if (gcount == BUFFSIZE) {
            tr1 = buffer[BUFFSIZE - 2];
            tr2 = buffer[BUFFSIZE - 1];
        }
        for (int i = 0; i < gcount - 3 + 1; ++i) {
            int ans = makeTrig(buffer[i], buffer[i + 1], buffer[i + 2]);
            //set.insert(ans);
            file.trigrams.insert(ans);
        }
    }
}