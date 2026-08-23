#include "upload_metrics.h"

#include <assert.h>
#include <math.h>

int main() {
    UploadMetricsTracker tracker;
    assert(!tracker.active());
    tracker.record_wind_sample(99.0f);
    assert(tracker.gust_10m_kmh() == 0.0f);

    tracker.begin(100, 1000);
    tracker.set_day(2026235, 4);
    assert(tracker.observe_total_tips(103) == 3);
    assert(tracker.rain_today_tips() == 7);
    assert(tracker.observe_total_tips(2) == 0); // Existing counter was manually reset.
    assert(tracker.rain_today_tips() == 7);

    tracker.record_wind_sample(20.0f);
    tracker.advance_time(61000);
    tracker.record_wind_sample(10.0f);
    tracker.advance_time(541000);
    assert(fabsf(tracker.gust_10m_kmh() - 20.0f) < 0.01f);
    tracker.advance_time(661000);
    assert(fabsf(tracker.gust_10m_kmh() - 10.0f) < 0.01f);
    tracker.advance_time(721000);
    assert(tracker.gust_10m_kmh() == 0.0f);

    assert(tracker.update_day(2026236));
    assert(tracker.rain_today_tips() == 0);
    assert(!tracker.update_day(2026236));
    return 0;
}
