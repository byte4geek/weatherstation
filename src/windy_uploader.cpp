#include "windy_uploader.h"

#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "globals.h"
#include "windy_formatter.h"

WindyUploadResult upload_windy_observation(const String& station_id,
                                           const String& station_password,
                                           const WeatherObservation& observation) {
    WindyUploadResult result = {false, 0, "connection failed"};
    char query[512];
    if (!build_windy_query(query, sizeof(query), station_id.c_str(),
                           FIRMWARE_VERSION, observation)) {
        result.message = "unable to format observation";
        return result;
    }

    BearSSL::WiFiClientSecure client;
    client.setInsecure();
    client.setBufferSizes(1024, 512);
    client.setTimeout(5000);
    HTTPClient http;
    http.setTimeout(5000);
    http.setUserAgent(String("byte4geek-weatherstation/") + FIRMWARE_VERSION);
    String url("https://stations.windy.com/api/v2/observation/update");
    url += query;
    if (!http.begin(client, url)) {
        result.message = "unable to initialize HTTPS client";
        return result;
    }
    http.addHeader("Authorization", String("Bearer ") + station_password);
    result.http_status = http.GET();
    http.getString();
    http.end();

    result.success = result.http_status >= 200 && result.http_status < 300;
    if (result.success) result.message = "success";
    else if (result.http_status <= 0) result.message = "network error";
    else if (result.http_status == 401 || result.http_status == 403) result.message = "station password rejected";
    else if (result.http_status == 429) result.message = "upload interval too short";
    else result.message = String("HTTP ") + result.http_status;
    app_log("[Weather Services] Windy upload %s (HTTP %d).",
            result.success ? "succeeded" : "failed", result.http_status);
    return result;
}
