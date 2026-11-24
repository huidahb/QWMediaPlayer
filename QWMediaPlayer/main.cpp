// ============================================================================
// main.cpp —— 程序入口
// ============================================================================

#include <QtWidgets/QApplication>
#include "MainWidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWidget window;
    window.show();

    return app.exec();
}
