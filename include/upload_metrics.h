#pragma once

#include <stdint.h>

class UploadMetricsTracker {
public:
    UploadMetricsTracker();

    void begin(uint32_t current_total_tips, unsigned long now_ms);
    void set_day(int day_key, uint32_t restored_today_tips);
    bool update_day(int day_key);
    uint32_t observe_total_tips(uint32_t current_total_tips);
    void record_wind_sample(float instant_speed_kmh);
    void advance_time(unsigned long now_ms);

    bool active() const;
    int day_key() const;
    uint32_t rain_today_tips() const;
    float gust_10m_kmh() const;

private:
    void rotate_gust_minute();
    void recalculate_gust();

    bool active_;
    int day_key_;
    uint32_t last_total_tips_;
    uint32_t rain_today_tips_;
    unsigned long last_minute_ms_;
    float gust_minute_history_[10];
    uint8_t gust_minute_index_;
    float gust_current_minute_kmh_;
    float gust_10m_kmh_;
};
