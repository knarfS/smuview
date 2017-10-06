#include "smuview.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    smuview w;
    w.show();

    return app.exec();
}

