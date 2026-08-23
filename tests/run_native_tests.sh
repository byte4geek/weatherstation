#!/usr/bin/env sh
set -eu

c++ -std=c++11 -Wall -Wextra -Werror \
  -Iinclude \
  src/weather_observation.cpp \
  tests/test_weather_observation.cpp \
  -o /tmp/weatherstation-observation-tests

/tmp/weatherstation-observation-tests

c++ -std=c++11 -Wall -Wextra -Werror \
  -Iinclude \
  tests/test_weather_service_features.cpp \
  -o /tmp/weatherstation-feature-default-tests

/tmp/weatherstation-feature-default-tests

c++ -std=c++11 -Wall -Wextra -Werror \
  -Iinclude \
  -D TEST_MULTIPLE_WEATHER_SERVICES=1 \
  -D WEATHER_UPLOAD_WUNDERGROUND=1 \
  -D WEATHER_UPLOAD_CWOP=1 \
  -D WEATHER_UPLOAD_WINDY=1 \
  tests/test_weather_service_features.cpp \
  -o /tmp/weatherstation-feature-multiple-tests

/tmp/weatherstation-feature-multiple-tests

c++ -std=c++11 -Wall -Wextra -Werror \
  -Iinclude \
  src/upload_metrics.cpp \
  tests/test_upload_metrics.cpp \
  -o /tmp/weatherstation-upload-metrics-tests

/tmp/weatherstation-upload-metrics-tests

c++ -std=c++11 -Wall -Wextra -Werror \
  -Iinclude \
  src/cwop_formatter.cpp \
  tests/test_cwop_formatter.cpp \
  -o /tmp/weatherstation-cwop-tests

/tmp/weatherstation-cwop-tests

c++ -std=c++11 -Wall -Wextra -Werror \
  -Iinclude \
  src/weathercloud_formatter.cpp \
  tests/test_weathercloud_formatter.cpp \
  -o /tmp/weatherstation-weathercloud-tests

/tmp/weatherstation-weathercloud-tests
