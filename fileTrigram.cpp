//
// Created by Елена on 09/01/2019.
//
#include "fileTrigram.h"

#include <utility>

fileTrigram::fileTrigram() = default;
fileTrigram::fileTrigram(QString name) {
    file = std::move(name);
}