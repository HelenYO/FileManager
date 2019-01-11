#ifndef MAINWINDOW2_H
#define MAINWINDOW2_H

#include <QMainWindow>
#include <memory>
#include "fileTrigram.h"

namespace Ui {
    class MainWindow1;
}

class subFind : public QMainWindow {
    Q_OBJECT

public:
    explicit subFind(QWidget *parent = nullptr);
    ~subFind() override;

private:
    void select_directory();
    void startPreprocessing();
    void addTrigrams(QString name, std::set<int> &set);
    void start_find();

private:
    std::unique_ptr<Ui::MainWindow1> ui;
    QString curDir;
    std::vector<fileTrigram> files;
    std::vector<std::pair<QString, std::vector<std::pair<int, int>>>> contains;

};

#endif // MAINWINDOW2_H


