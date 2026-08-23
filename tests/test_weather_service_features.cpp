#include "weather_service_features.h"

#if defined(TEST_MULTIPLE_WEATHER_SERVICES)
static_assert(WEATHER_UPLOAD_WUNDERGROUND == 1, "Wunderground should be compiled");
static_assert(WEATHER_UPLOAD_CWOP == 1, "CWOP should be compiled");
static_assert(WEATHER_UPLOAD_WINDY == 1, "Windy should be compiled");
static_assert(WEATHER_UPLOAD_PWSWEATHER == 0, "PWSWeather should remain excluded");
static_assert(WEATHER_UPLOAD_ANY, "At least one uploader should be compiled");
#else
static_assert(WEATHER_UPLOAD_WUNDERGROUND == 0, "Uploaders must default off");
static_assert(WEATHER_UPLOAD_PWSWEATHER == 0, "Uploaders must default off");
static_assert(WEATHER_UPLOAD_CWOP == 0, "Uploaders must default off");
static_assert(WEATHER_UPLOAD_WEATHERCLOUD == 0, "Uploaders must default off");
static_assert(WEATHER_UPLOAD_WINDY == 0, "Uploaders must default off");
static_assert(WEATHER_UPLOAD_AWEKAS == 0, "Uploaders must default off");
static_assert(WEATHER_UPLOAD_WOW_BE == 0, "Uploaders must default off");
static_assert(!WEATHER_UPLOAD_ANY, "No uploader should compile by default");
#endif

int main() {
    return 0;
}
