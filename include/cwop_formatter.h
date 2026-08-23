#pragma once

#include <stddef.h>

#include "weather_observation.h"

bool build_cwop_login(char* output,
                      size_t output_size,
                      const char* station_id,
                      const char* passcode,
                      const char* firmware_version);

bool build_cwop_packet(char* output,
                       size_t output_size,
                       const char* station_id,
                       double latitude,
                       double longitude,
                       const char* firmware_version,
                       const WeatherObservation& observation);
