#include "services/service.h"

namespace services {
namespace {
Service *g_services[kMaxServices] = {};
size_t g_count = 0;
} // namespace

bool registerService(Service &service) {
    if (g_count >= kMaxServices) {
        return false;
    }
    g_services[g_count++] = &service;
    return true;
}

void beginAll() {
    for (size_t i = 0; i < g_count; ++i) {
        g_services[i]->begin();
    }
}

void loopAll(uint32_t nowMs) {
    for (size_t i = 0; i < g_count; ++i) {
        g_services[i]->loop(nowMs);
    }
}

void refreshAll() {
    for (size_t i = 0; i < g_count; ++i) {
        g_services[i]->refresh();
    }
}

size_t count() { return g_count; }

Service *at(size_t index) { return index < g_count ? g_services[index] : nullptr; }

} // namespace services
