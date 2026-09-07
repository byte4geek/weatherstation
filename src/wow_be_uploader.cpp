#include "wow_be_uploader.h"

#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "globals.h"
#include "wow_be_formatter.h"

WowBeUploadResult upload_wow_be_observation(const String& site_id,
                                            const String& authentication_key,
                                            const WeatherObservation& observation) {
    WowBeUploadResult result = {false, 0, "connection failed"};
    char query[640];
    if (!build_wow_be_query(query, sizeof(query), site_id.c_str(), authentication_key.c_str(),
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
    String url("https://wow.meteo.be/api/v2/send");
    url += query;
    if (!http.begin(client, url)) {
        result.message = "unable to initialize HTTPS client";
        return result;
    }
    result.http_status = http.GET();
    http.getString();
    http.end();
    result.success = result.http_status >= 200 && result.http_status < 300;
    if (result.success) result.message = "success";
    else if (result.http_status <= 0) result.message = "network error";
    else if (result.http_status == 401 || result.http_status == 403) result.message = "site ID or key rejected";
    else result.message = String("HTTP ") + result.http_status;
    app_log("[Weather Services] WOW-BE upload %s (HTTP %d).",
            result.success ? "succeeded" : "failed", result.http_status);
    return result;
}
