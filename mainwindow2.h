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
    void addFileTrigramsToFiles(fileTrigram add);
    void finishThings();
    void addToFSWatcher(QString name);
    void increaseBar();
    void set_max_index_bar(int max);

private:
    void select_directory();
    void startPreprocess();
    void start_find();
    void interruption();
    void interruptionStart();
    void change(QString path);
    void interruptionTrig();

private:
    std::unique_ptr<Ui::mainwindow1> ui;
    QString curDir;
    std::vector<fileTrigram> files;
    QThread* thread = nullptr;
    QThread* threadTrig = nullptr;
    std::clock_t time;
    QFileSystemWatcher *fsWatcher;
    std::vector<QString> toChange;
};

#endif // MAINWINDOW2_H