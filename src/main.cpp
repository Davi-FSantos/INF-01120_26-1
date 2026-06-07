#include <QApplication>
#include "musicMachine.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MusicMachine w;
    w.show();

    return QApplication::exec();
}
