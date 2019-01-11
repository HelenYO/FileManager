//
// Created by Елена on 09/01/2019.
//
#include "fileTrigram.h"

fileTrigram::fileTrigram(QString name) {
    file = name;
    trigrams = *(new std::set<int>());
}