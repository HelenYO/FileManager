#include "finderOfStrings.h"

struct cancellation_exception : std::exception {
    const char *what() const noexcept override {
        return "Stop thread";
    }
};

finderSub::finderSub(QString dir, std::string strSub, std::vector<fileTrigram> filestemp) {
    curDir = std::move(dir);
    sub = std::move(strSub);
    files = std::move(filestemp);
}

finderSub::~finderSub() = default;

void finderSub::process() {
    scan_directory();
}

void finderSub::scan_directory() {
    try {

        auto cancellation_point = [thread = QThread::currentThread(), this]() {
            if (thread->isInterruptionRequested()) {
                throw cancellation_exception();
            }
        };

        std::vector<uint32_t> subTrig;
        if (sub.size() > 2) {
            for (int i = 0; i < sub.size() - 3 + 1; ++i) {
                cancellation_point();
                uint32_t trig = 0;
                auto a = (uint8_t) sub[i];
                trig |= a;
                trig <<= 8;
                a = (uint8_t) sub[i + 1];
                trig |= a;
                trig <<= 8;
                a = (uint8_t) sub[i + 2];
                trig |= a;

                subTrig.push_back(trig);
            }
        }

        cancellation_point();

        for (auto &file : files) {
            cancellation_point();
            bool contain = true;
            for (uint32_t j : subTrig) {
                if (file.trigrams.end() == file.trigrams.find(j)) {
                    contain = false;
                    break;
                }
            }
            if (contain) {
                std::ifstream fin(file.file.toStdString(), std::ios::binary);
                std::string text;
                std::string pat = sub;
                int number = 0;
                std::vector<std::pair<int, int>> thisLine;

                while (!fin.eof()) {
                    cancellation_point();
                    number++;
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

                if (!thisLine.empty()) {
                    emit addToTree({file.file, thisLine});
                }
            }
            emit updateProgressBar();
        }
        emit finished();
    } catch (std::exception &ex) {
        emit error();
    }
}