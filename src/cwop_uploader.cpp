#include "cwop_uploader.h"

#include <ESP8266WiFi.h>

#include "cwop_formatter.h"
#include "globals.h"

CwopUploadResult upload_cwop_observation(const String& server,
                                         uint16_t port,
                                         const String& station_id,
                                         const String& passcode,
                                         double latitude,
                                         double longitude,
                                         const WeatherObservation& observation) {
    CwopUploadResult result = {false, "connection failed"};
    char login[128];
    char packet[256];
    if (!build_cwop_login(login, sizeof(login), station_id.c_str(),
                          passcode.c_str(), FIRMWARE_VERSION) ||
        !build_cwop_packet(packet, sizeof(packet), station_id.c_str(),
                           latitude, longitude, FIRMWARE_VERSION, observation)) {
        result.message = "invalid CWOP settings or observation";
        return result;
    }

    WiFiClient client;
    client.setTimeout(5000);
    if (!client.connect(server.c_str(), port)) {
        app_log("[Weather Services] CWOP connection to %s:%u failed.",
                server.c_str(), port);
        return result;
    }

    const size_t login_length = strlen(login);
    const size_t packet_length = strlen(packet);
    bool written = client.write(reinterpret_cast<const uint8_t*>(login), login_length) == login_length &&
                   client.write(reinterpret_cast<const uint8_t*>(packet), packet_length) == packet_length;
    client.flush();

    unsigned long wait_started = millis();
    String response;
    while (millis() - wait_started < 1000UL) {
        while (client.available()) response += static_cast<char>(client.read());
        if (response.indexOf("logresp") >= 0) break;
        delay(10);
        yield();
    }
    client.stop();

    result.success = written && response.indexOf("invalid") < 0;
    result.message = result.success ? "packet sent" : "server rejected login or write failed";
    app_log("[Weather Services] CWOP upload %s.", result.success ? "succeeded" : "failed");
    return result;
}
