#pragma once
#include "kits/required/controller_base/HttpControllerBase.h"
namespace _Controllers
{
    class HttpController : public HttpControllerBase<HttpController>
    {
      public:
        QHttpServerResponse onSelect(const QHttpServerRequest &);
        HTTP_LIST_BEGIN
        HTTP_ADD(TIS_Info::HttpService::HttpRoutes::database_select, HttpController::onSelect);
        HTTP_LIST_END
    };
} // namespace _Controllers
