#include "awekas_uploader.h"

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Hash.h>

#include "awekas_formatter.h"
#include "globals.h"

AwekasUploadResult upload_awekas_observation(const String& username,
                                             const String& password,
                                             float latitude,
                                             float longitude,
                                             const WeatherObservation& observation) {
    AwekasUploadResult result = {false, 0, "connection failed"};
    MD5Builder md5;
    md5.begin();
    md5.add(password);
    md5.calculate();
    String password_hash = md5.toString();

    char query[640];
    if (!build_awekas_query(query, sizeof(query), username.c_str(), password_hash.c_str(),
                            latitude, longitude, FIRMWARE_VERSION, observation)) {
        result.message = "unable to format observation";
        return result;
    }

    WiFiClient client;
    client.setTimeout(5000);
    HTTPClient http;
    http.setTimeout(5000);
    http.setUserAgent(String("byte4geek-weatherstation/") + FIRMWARE_VERSION);
    String url("http://data.awekas.at/eingabe_pruefung.php");
    url += query;
    if (!http.begin(client, url)) {
        result.message = "unable to initialize HTTP client";
        return result;
    }
    result.http_status = http.GET();
    String response = http.getString();
    response.trim();
    http.end();

    result.success = result.http_status >= 200 && result.http_status < 300 && response.startsWith("OK");
    if (result.success) result.message = "success";
    else if (result.http_status <= 0) result.message = "network error";
    else if (response.indexOf("Benutzer/Passwort Fehler") >= 0) result.message = "username or password rejected";
    else result.message = String("HTTP ") + result.http_status;
    app_log("[Weather Services] AWEKAS upload %s (HTTP %d).",
            result.success ? "succeeded" : "failed", result.http_status);
    return result;
}
