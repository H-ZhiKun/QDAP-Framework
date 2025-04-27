#include "HttpController.h"
#include "kits/required/log/CRossLogger.h"
#include <chrono>

using namespace _Controllers;
using namespace _Kits;
QHttpServerResponse HttpController::onSelect(const QHttpServerRequest &req)
{
    _Kits::LogInfo("recv onSelect");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return QHttpServerResponse(req.body());
}
