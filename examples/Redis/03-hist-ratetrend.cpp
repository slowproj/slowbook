
#include <iostream>
#include <slowbook.hpp>
#include <slowbook/datastore_Redis.hpp>

namespace sb = slowbook;


int main(void)
{
    sb::RandomPulse random_pulse(/*rate_cps*/ 30);
    
    sb::RateTrend rate_trend(/*tick_s*/ 1);
    sb::Histogram charge_hist(/*nbins*/ 100, /*min*/ 0, /*max*/ 100);
    
    sb::DataStore_Redis ds("redis://localhost/1");
    sb::SimpleNumericSchema numeric_schema("Numeric");
    sb::SimpleObjectSchema object_schema("Objects");
        
    long t0 = time(NULL);    
    while (true) {
        long t = time(NULL);
        double q = random_pulse.get();
        
        rate_trend.fill(t);
        charge_hist.fill(q);

        if (t >= t0 + 1.0) {
            ds.append(sb::SlowDashDataFrame(numeric_schema).tag("rate") << rate_trend);
            ds.update(sb::SlowDashDataFrame(object_schema).tag("charge") << charge_hist);
            
            std::cout << t << " " << q << std::endl;
            rate_trend.truncate();  // remove the points already recorded
            t0 = t;
        }
    }
    
    return 0;
}
