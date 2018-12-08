#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QDir>

namespace Ui {
class MainWindow;
}

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit main_window(QWidget *parent = 0);
    ~main_window();

private slots:
    void select_directory();
    void scan_directory();
    void find_copies(QVector<std::pair<QString, int>> vec , std::vector<std::ifstream> &streams, int degree);
    void show_about_dialog();
    void select_useless();
    void delete_useless();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QString curDir;

    bool wasDuplicate = false;
    qint64 sum = 0;
    qint64 sumProgress = 0;
    qint64 sumProgressAll = 0;
};

#endif // MAINWINDOW_H
