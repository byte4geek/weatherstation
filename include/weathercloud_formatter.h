#pragma once

#include <stddef.h>

#include "weather_observation.h"

bool build_weathercloud_query(char* output,
                              size_t output_size,
                              const char* device_id,
                              const char* device_key,
                              const char* firmware_version,
                              const WeatherObservation& observation);
