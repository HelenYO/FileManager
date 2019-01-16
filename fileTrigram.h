#ifndef FILEMANAGER_FILETRIGRAM_H
#define FILEMANAGER_FILETRIGRAM_H

#include <QString>
#include <vector>
#include <set>
#include <unordered_set>

struct fileTrigram {

public:
    fileTrigram();

    explicit fileTrigram(QString name);
public:
    QString file;
    std::unordered_set<int> trigrams;
};

#endif //FILEMANAGER_FILETRIGRAM_H