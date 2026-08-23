#pragma once

#include <stddef.h>

#include "weather_observation.h"

bool build_awekas_query(char* output,
                        size_t output_size,
                        const char* username,
                        const char* password_md5,
                        double latitude,
                        double longitude,
                        const char* firmware_version,
                        const WeatherObservation& observation);
