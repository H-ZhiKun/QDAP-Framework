#pragma once
namespace TIS_Info
{
    namespace QmlCommunication
    {
        namespace Radar
        {
            constexpr char stateSendQml[] = "stateSendQml";
            constexpr char dataSendQml[] = "dataSendQml";
            constexpr char overInfoSendQml[] = "overInfoSendQml";
            constexpr char overConfigSendQml[] = "overConfigSendQml";
        } // namespace Radar
        constexpr char strForQmlSignals[] = "ForQmlSignals";
        constexpr char strData[] = "data";
    } // namespace QmlCommunication

    namespace HttpService
    {
        namespace HttpRoutes
        {
            constexpr char database_select[] = "/database/select";
            constexpr char database_insert[] = "/database/insert";
            constexpr char api_communication[] = "/api/communication";
        } // namespace HttpRoutes
    } // namespace HttpService
} // namespace TIS_Info