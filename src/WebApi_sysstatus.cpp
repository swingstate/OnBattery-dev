// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2024 Thomas Basler and others
 */
#include "WebApi_sysstatus.h"
#include "Configuration.h"
#include "NetworkSettings.h"
#include "PinMapping.h"
#include "WebApi.h"
#include <AsyncJson.h>
#include <Hoymiles.h>
#include <LittleFS.h>
#include <ResetReason.h>
#include "__compiled_constants.h"

void WebApiSysstatusClass::init(AsyncWebServer& server, Scheduler& scheduler)
{
    using std::placeholders::_1;

    server.on("/api/system/status", HTTP_GET, std::bind(&WebApiSysstatusClass::onSystemStatus, this, _1));
}

void WebApiSysstatusClass::onSystemStatus(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentialsReadonly(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    auto& root = response->getRoot();

    root["hostname"] = NetworkSettings.getHostname();

    root["sdkversion"] = ESP.getSdkVersion();
    root["cpufreq"] = ESP.getCpuFreqMHz();

    root["heap_total"] = ESP.getHeapSize();
    root["heap_used"] = ESP.getHeapSize() - ESP.getFreeHeap();
    root["heap_max_block"] = ESP.getMaxAllocHeap();
    root["heap_min_free"] = ESP.getMinFreeHeap();
    root["psram_total"] = ESP.getPsramSize();
    root["psram_used"] = ESP.getPsramSize() - ESP.getFreePsram();
    root["sketch_total"] = ESP.getFreeSketchSpace();
    root["sketch_used"] = ESP.getSketchSize();
    root["littlefs_total"] = LittleFS.totalBytes();
    root["littlefs_used"] = LittleFS.usedBytes();

    root["chiprevision"] = ESP.getChipRevision();
    root["chipmodel"] = ESP.getChipModel();
    root["chipcores"] = ESP.getChipCores();
    root["flashsize"] = ESP.getFlashChipSize();

    String reason;
    reason = ResetReason::get_reset_reason_verbose(0);
    root["resetreason_0"] = reason;

    reason = ResetReason::get_reset_reason_verbose(1);
    root["resetreason_1"] = reason;

    root["cfgsavecount"] = Configuration.get().Cfg.SaveCount;

    char version[16];
    snprintf(version, sizeof(version), "%d.%d.%d", CONFIG_VERSION >> 24 & 0xff, CONFIG_VERSION >> 16 & 0xff, CONFIG_VERSION >> 8 & 0xff);
    root["config_version"] = version;
    root["git_hash"] = __COMPILED_GIT_HASH__;
    root["git_branch"] = __COMPILED_GIT_BRANCH__;
    root["pioenv"] = PIOENV;

    root["uptime"] = esp_timer_get_time() / 1000000;

    root["nrf_configured"] = PinMapping.isValidNrf24Config();
    root["nrf_connected"] = Hoymiles.getRadioNrf()->isConnected();
    root["nrf_pvariant"] = Hoymiles.getRadioNrf()->isPVariant();

    root["cmt_configured"] = PinMapping.isValidCmt2300Config();
    root["cmt_connected"] = Hoymiles.getRadioCmt()->isConnected();

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}
