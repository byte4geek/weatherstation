#pragma once

#include <stddef.h>

#include "weather_observation.h"

bool build_wow_be_query(char* output,
                        size_t output_size,
                        const char* site_id,
                        const char* authentication_key,
                        const char* firmware_version,
                        const WeatherObservation& observation);
