#include "service/AppFramework.h"
#include <QSharedMemory>

int main(int argc, char *argv[])
{
    QSharedMemory sharedMemory("togee_infinity_station");
    if (sharedMemory.attach())
    {
        qDebug() << "Another instance is already running!";
        std::terminate();
    }

    if (!sharedMemory.create(1))
    {
        qDebug() << "Failed to create shared memory!";
        std::terminate();
    }
    auto ret = _Service::App().run(argc, argv);
    return ret;
}
