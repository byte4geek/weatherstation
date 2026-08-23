#include "wu_uploader.h"

#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "globals.h"

namespace {
void append_parameter(String& query, const char* name, const String& value) {
    query += '&';
    query += name;
    query += '=';
    query += url_encode(value);
}

String value(float number, unsigned int decimals) {
    return String(number, decimals);
}
}

String url_encode(const String& input) {
    static const char hex[] = "0123456789ABCDEF";
    String encoded;
    encoded.reserve(input.length() * 3);
    for (size_t i = 0; i < input.length(); ++i) {
        const uint8_t c = static_cast<uint8_t>(input[i]);
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' ||
            c == '.' || c == '~') {
            encoded += static_cast<char>(c);
        } else {
            encoded += '%';
            encoded += hex[c >> 4];
            encoded += hex[c & 0x0F];
        }
    }
    return encoded;
}

String build_wu_query(const String& station_id,
                      const String& station_key,
                      const WeatherObservation& observation) {
    String query;
    query.reserve(400);
    query = "?ID=" + url_encode(station_id) + "&PASSWORD=" + url_encode(station_key);
    append_parameter(query, "dateutc", "now");

    if (observation.has_wind_direction) {
        append_parameter(query, "winddir", value(observation.wind_direction_deg, 0));
    }
    if (observation.has_wind_speed) {
        append_parameter(query, "windspeedmph", value(observation.wind_speed_kmh * 0.621371f, 2));
    }
    if (observation.has_wind_gust) {
        append_parameter(query, "windgustmph", value(observation.wind_gust_10m_kmh * 0.621371f, 2));
    }
    if (observation.has_temperature) {
        append_parameter(query, "tempf", value(observation.temperature_c * 1.8f + 32.0f, 2));
    }
    if (observation.has_humidity) {
        append_parameter(query, "humidity", value(observation.humidity_pct, 1));
    }
    if (observation.has_dew_point) {
        append_parameter(query, "dewptf", value(observation.dew_point_c * 1.8f + 32.0f, 2));
    }
    if (observation.has_pressure) {
        append_parameter(query, "baromin", value(observation.pressure_hpa * 0.029529983f, 4));
    }
    if (observation.has_rain_hour) {
        append_parameter(query, "rainin", value(observation.rain_hour_mm / 25.4f, 3));
    }
    if (observation.has_rain_today) {
        append_parameter(query, "dailyrainin", value(observation.rain_today_mm / 25.4f, 3));
    }

    append_parameter(query, "softwaretype", String("byte4geek-weatherstation-") + FIRMWARE_VERSION);
    append_parameter(query, "action", "updateraw");
    return query;
}

WuUploadResult upload_wu_observation(const char* service_name,
                                     const char* base_url,
                                     const String& station_id,
                                     const String& station_key,
                                     const WeatherObservation& observation) {
    WuUploadResult result = {false, 0, "connection failed"};
    BearSSL::WiFiClientSecure client;
    // ESP8266 has no maintained system CA store. Encryption is still used, but
    // certificate verification remains disabled until a configurable trust
    // anchor or CertStore is added.
    client.setInsecure();
    client.setTimeout(5000);

    String url(base_url);
    url += build_wu_query(station_id, station_key, observation);

    HTTPClient http;
    http.setTimeout(5000);
    http.setUserAgent(String("byte4geek-weatherstation/") + FIRMWARE_VERSION);
    if (!http.begin(client, url)) {
        result.message = "unable to initialize HTTPS client";
        return result;
    }

    result.http_status = http.GET();
    String response = http.getString();
    response.trim();
    http.end();

    String lower = response;
    lower.toLowerCase();
    result.success = result.http_status >= 200 && result.http_status < 300 &&
                     (lower.length() == 0 || lower.indexOf("success") >= 0);
    if (result.success) {
        result.message = "success";
    } else if (result.http_status <= 0) {
        result.message = "network error";
    } else {
        result.message = String("HTTP ") + result.http_status;
    }

    app_log("[Weather Services] %s upload %s (HTTP %d).",
            service_name, result.success ? "succeeded" : "failed", result.http_status);
    return result;
}
