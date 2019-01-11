#include "mainwindow.h"
#include "mainwindow2.h"
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    main_window w;
//    w.show();
    subFind w2;
    w2.show();

//    for (int i = 0 ; i < 0; i++) {
//        std::cout << "aa ";
//    }

    return a.exec();
}
