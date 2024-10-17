#include "MHGUQuestEditor.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MHGUQuestEditor w;
    w.show();
    return a.exec();
}
