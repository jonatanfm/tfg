
#include <QApplication>

#include "MainWindow.h"

// Program entry point
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    /*#if defined(_WIN32)
        AllocConsole(); // Open a console
    #endif*/

    int returnValue;
    {
        MainWindow* window = new MainWindow();
        window->show();

        returnValue = app.exec();

        window->close();
        delete window;
    }

    return returnValue;
}
