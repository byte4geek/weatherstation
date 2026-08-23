#include "upload_metrics.h"

#include <string.h>

UploadMetricsTracker::UploadMetricsTracker()
    : active_(false),
      day_key_(-1),
      last_total_tips_(0),
      rain_today_tips_(0),
      last_minute_ms_(0),
      gust_minute_index_(0),
      gust_current_minute_kmh_(0.0f),
      gust_10m_kmh_(0.0f) {
    memset(gust_minute_history_, 0, sizeof(gust_minute_history_));
}

void UploadMetricsTracker::begin(uint32_t current_total_tips, unsigned long now_ms) {
    active_ = true;
    day_key_ = -1;
    last_total_tips_ = current_total_tips;
    rain_today_tips_ = 0;
    last_minute_ms_ = now_ms;
    gust_minute_index_ = 0;
    gust_current_minute_kmh_ = 0.0f;
    gust_10m_kmh_ = 0.0f;
    memset(gust_minute_history_, 0, sizeof(gust_minute_history_));
}

void UploadMetricsTracker::set_day(int day_key, uint32_t restored_today_tips) {
    day_key_ = day_key;
    rain_today_tips_ = restored_today_tips;
}

bool UploadMetricsTracker::update_day(int day_key) {
    if (day_key_ == day_key) return false;
    day_key_ = day_key;
    rain_today_tips_ = 0;
    return true;
}

uint32_t UploadMetricsTracker::observe_total_tips(uint32_t current_total_tips) {
    if (!active_) return 0;
    if (current_total_tips < last_total_tips_) {
        last_total_tips_ = current_total_tips;
        return 0;
    }
    const uint32_t difference = current_total_tips - last_total_tips_;
    last_total_tips_ = current_total_tips;
    if (day_key_ >= 0) rain_today_tips_ += difference;
    return difference;
}

void UploadMetricsTracker::record_wind_sample(float instant_speed_kmh) {
    if (!active_ || instant_speed_kmh < 0.0f) return;
    if (instant_speed_kmh > gust_current_minute_kmh_) {
        gust_current_minute_kmh_ = instant_speed_kmh;
    }
    if (instant_speed_kmh > gust_10m_kmh_) gust_10m_kmh_ = instant_speed_kmh;
}

void UploadMetricsTracker::advance_time(unsigned long now_ms) {
    if (!active_) return;
    unsigned long elapsed = now_ms - last_minute_ms_;
    if (elapsed >= 600000UL) {
        memset(gust_minute_history_, 0, sizeof(gust_minute_history_));
        gust_minute_index_ = 0;
        gust_current_minute_kmh_ = 0.0f;
        gust_10m_kmh_ = 0.0f;
        last_minute_ms_ = now_ms;
        return;
    }
    while (elapsed >= 60000UL) {
        rotate_gust_minute();
        last_minute_ms_ += 60000UL;
        elapsed -= 60000UL;
    }
}

void UploadMetricsTracker::rotate_gust_minute() {
    gust_minute_history_[gust_minute_index_] = gust_current_minute_kmh_;
    gust_minute_index_ = (gust_minute_index_ + 1) % 10;
    gust_current_minute_kmh_ = 0.0f;
    recalculate_gust();
}

void UploadMetricsTracker::recalculate_gust() {
    gust_10m_kmh_ = gust_current_minute_kmh_;
    for (uint8_t i = 0; i < 10; ++i) {
        if (gust_minute_history_[i] > gust_10m_kmh_) {
            gust_10m_kmh_ = gust_minute_history_[i];
        }
    }
}

bool UploadMetricsTracker::active() const { return active_; }
int UploadMetricsTracker::day_key() const { return day_key_; }
uint32_t UploadMetricsTracker::rain_today_tips() const { return rain_today_tips_; }
float UploadMetricsTracker::gust_10m_kmh() const { return gust_10m_kmh_; }
