
#include <iostream>
#include <slowbook.hpp>
#include <slowbook/datastore_Redis.hpp>

namespace sb = slowbook;


int main(void)
{
    sb::RandomPulse random_pulse(/*rate_cps*/ 10);
    
    sb::Histogram charge_hist(/*nbins*/ 100, /*min*/ 0, /*max*/ 100);
    
    sb::DataStore_Redis ds("redis://localhost:6379/1");
    sb::SimpleObjectSchema object_schema("Objects");
        
    long t0 = time(NULL);
    while (true) {
        long t = time(NULL);
        double q = random_pulse.get();
        
        charge_hist.fill(q);

        if (t >= t0 + 1.0) {            
            std::cout << t << " " << q << std::endl;
            t0 = t;

            // Here "update()" is used instead of "append()", to keep only the last objects //
            ds.update(sb::SlowDashDataFrame(object_schema).tag("charge") << charge_hist);
        }
    }
    
    return 0;
}
