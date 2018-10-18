#include "openrasp_v8.h"

namespace openrasp
{
using namespace std;

std::mutex Platform::mtx;
v8::Platform *Platform::platform = nullptr;

void Platform::Initialize()
{
    lock_guard<mutex> lock(mtx);
    if (!platform)
    {
        platform = v8::platform::CreateDefaultPlatform(1);
        v8::V8::InitializePlatform(platform);
    }
}

void Platform::Shutdown()
{
    lock_guard<mutex> lock(mtx);
    if (platform)
    {
        v8::V8::ShutdownPlatform();
        delete platform;
        platform = nullptr;
    }
}
} // namespace openrasp