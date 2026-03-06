
#include <iostream>
#include <slowbook.hpp>

namespace sb = slowbook;


int main(void)
{
    sb::RandomPulse random_pulse(/*rate_cps*/ 30);
    
    sb::RateTrend rate_trend(/*tick_s*/ 1);
    
    sb::DataStore_JsonDump ds;
    //sb::DataStore_CsvFile ds("SlowStore_");
    
    sb::SimpleNumericSchema schema("NumericData");

    long t0 = time(NULL);    
    while (true) {
        double q = random_pulse.get();
        long t = time(NULL);
        
        rate_trend.fill(t);

        if (t >= t0 + 1.0) {
            ds.append(sb::SlowDashDataFrame(schema).tag("rate").time(t) << rate_trend);
            rate_trend.truncate();  // remove the points already recorded
            
            std::cout << t << " " << q << std::endl;
            t0 = t;
        }
    }
    
    return 0;
}
