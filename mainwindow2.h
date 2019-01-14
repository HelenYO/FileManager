#ifndef MAINWINDOW2_H
#define MAINWINDOW2_H

#include <QMainWindow>
#include <memory>
#include <QFileSystemWatcher>
#include "fileTrigram.h"

namespace Ui {
    class mainwindow1;
}

class subFind : public QMainWindow {
    Q_OBJECT

public:
    explicit subFind(QWidget *parent = nullptr);
    ~subFind() override;

private slots:

    void addToTreeUI(std::pair<QString, std::vector<std::pair<int, int>>> add);
    void doFinishThings();
    void updBar();
    void changed(QString path);

private:
    void select_directory();
    void startPreprocessing();
    void addTrigrams(QString name, std::unordered_set<int> &set);
    void start_find();
    void interruption();
    void interruptionStart();
    bool check(QString name);

private:
    std::unique_ptr<Ui::mainwindow1> ui;
    QString curDir;
    std::vector<fileTrigram> files;
    //std::vector<std::pair<QString, std::vector<std::pair<int, int>>>> contains;
    QThread* thread = nullptr;
    std::clock_t time;
    QFileSystemWatcher *fsWatcher;
};

#endif // MAINWINDOW2_H


