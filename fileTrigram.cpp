#include "fileTrigram.h"

#include <utility>

fileTrigram::fileTrigram() = default;
fileTrigram::fileTrigram(QString name) {
    file = std::move(name);
}