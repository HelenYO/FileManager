//
// Created by Елена on 11/01/2019.
//

#include "finderOfStrings.h"
#include <array>

struct cancellation_exception : std::exception {
    const char *what() const noexcept override {
        return "Stop thread";
    }
};


finderSub::finderSub(QString dir, std::string strSub, std::vector<fileTrigram> filestemp) {
    curDir = dir;
    sub = strSub;
    files = filestemp;
}

finderSub::~finderSub() = default;

void finderSub::process() {
    scan_directory();
}

void finderSub::scan_directory() {
    std::vector<int> subTrig;
    for (int i = 0; i < sub.size() - 3 + 1; ++i) {
        int trig = 0;
        uint8_t a = (uint8_t)sub[i];
        trig |= a;
        trig <<= 8;
        a = (uint8_t)sub[i + 1];
        trig |= a;
        trig <<= 8;
        a = (uint8_t)sub[i + 2];
        trig |= a;

        std::cout << trig << " ";
        subTrig.push_back(trig);
    }

    //contains = *(new std::vector<std::pair<QString, std::vector<std::pair<int, int>>>>());

    for(int i = 0; i < files.size(); ++i) {
        bool contain = true;
        for (int j = 0; j < subTrig.size(); ++j) {
            if (files[i].trigrams.end() == files[i].trigrams.find(subTrig[j])) {
                contain = false;
                break;
            }
        }
        if(contain) {
            std::ifstream fin(files[i].file.toStdString(), std::ios::binary);
            std::string text;
            std::string pat = sub;
            int number = 0;
            std::vector<std::pair<int, int>> thisLine;

            while (!fin.eof()) {
                number++;//todo: я пока храню номер строки и склько в ней вхождений, я пушу в вектор строку, если в ней вхождений  нет- попаю
                std::getline(fin, text);

                int ans = 0;
                auto it = std::search(text.begin(), text.end(),
                                      std::boyer_moore_searcher(
                                              pat.begin(), pat.end()));
                while (it != text.end()) {
                    ans++;

                    it = std::search(it + 1, text.end(),
                                     std::boyer_moore_searcher(
                                             pat.begin(), pat.end()));
                }
                if (ans != 0) {
                    thisLine.emplace_back(number, ans);
                }
            }

            if (thisLine.size() != 0) {
                //contains.push_back({files[i].file, thisLine});
                emit addToTree({files[i].file, thisLine});
            }
        }
    }


    emit finished();
}