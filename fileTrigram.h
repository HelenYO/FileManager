//
// Created by Елена on 09/01/2019.
//

#ifndef FILEMANAGER_FILETRIGRAM_H
#define FILEMANAGER_FILETRIGRAM_H

#include <QString>
#include <vector>
#include <set>

struct fileTrigram {

public:
    fileTrigram(QString name);
public:
    QString file;
    std::set<int> trigrams;
};

#endif //FILEMANAGER_FILETRIGRAM_H
