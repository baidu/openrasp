#include "openrasp_config.h"

namespace openrasp
{
OpenraspConfig::OpenraspConfig(string &config, FromType type)
    : OpenraspConfig()
{
    switch (type)
    {
    case FromType::json:
        FromJson(config);
        break;
    case FromType::ini:
        FromIni(config);
        break;
    default:
        break;
    }
}
void OpenraspConfig::FromJson(string &json)
{

}
void OpenraspConfig::FromIni(string &json)
{

}
} // namespace openrasp