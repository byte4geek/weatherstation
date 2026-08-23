#pragma once

#include <stddef.h>

#include "weather_observation.h"

bool build_windy_query(char* output,
                       size_t output_size,
                       const char* station_id,
                       const char* firmware_version,
                       const WeatherObservation& observation);
