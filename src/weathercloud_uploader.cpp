#include "weathercloud_uploader.h"

#if defined(ESP32)
#include <HTTPClient.h>
#else
#include <ESP8266HTTPClient.h>
#endif
#if defined(ESP32)
#include <WiFiClientSecure.h>
#else
#include <WiFiClientSecureBearSSL.h>
#endif

#include "globals.h"
#include "weathercloud_formatter.h"

namespace {
const char* WEATHERCloud_URL = "https://api.weathercloud.net/v01/set";
}

WeathercloudUploadResult upload_weathercloud_observation(
    const String& device_id,
    const String& device_key,
    const WeatherObservation& observation) {
    WeathercloudUploadResult result = {false, 0, "connection failed"};
    char query[512];
    if (!build_weathercloud_query(query, sizeof(query), device_id.c_str(), device_key.c_str(),
                                  FIRMWARE_VERSION, observation)) {
        result.message = "unable to format observation";
        return result;
    }

    #if defined(ESP32)
    WiFiClientSecure client;
#else
    BearSSL::WiFiClientSecure client;
#endif
    client.setInsecure();
    #if !defined(ESP32)
        client.setBufferSizes(1024, 512);
#endif
    client.setTimeout(5000);
    HTTPClient http;
    http.setTimeout(5000);
    http.setUserAgent(String("byte4geek-weatherstation/") + FIRMWARE_VERSION);
    String url(WEATHERCloud_URL);
    url += query;
    if (!http.begin(client, url)) {
        result.message = "unable to initialize HTTPS client";
        return result;
    }

    result.http_status = http.GET();
    String response = http.getString();
    response.trim();
    http.end();
    result.success = result.http_status >= 200 && result.http_status < 300 &&
                     (response.length() == 0 || response == "200");
    if (result.success) result.message = "success";
    else if (result.http_status <= 0) result.message = "network error";
    else if (result.http_status == 401) result.message = "device ID or key rejected";
    else if (result.http_status == 429) result.message = "upload interval too short";
    else result.message = String("HTTP ") + result.http_status;

    app_log("[Weather Services] Weathercloud upload %s (HTTP %d).",
            result.success ? "succeeded" : "failed", result.http_status);
    return result;
}
