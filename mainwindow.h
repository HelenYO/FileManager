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
    void show_about_dialog();

    void write_dfs(QDir const& d);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QString curDir;
};

#endif // MAINWINDOW_H
