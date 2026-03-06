
#include <iostream>
#include <slowbook.hpp>

namespace sb = slowbook;


int main(void)
{
    sb::RandomPulse random_pulse(/*rate_cps*/ 10);
    
    sb::Histogram hist(/*nbins*/ 100, /*min*/ 0, /*max*/ 100);
    
    sb::DataStore_CsvFile ds("SlowStore_");
    //sb::DataStore_JsonDump ds;
    
    sb::SimpleObjectSchema schema("Objects");

    long t0 = time(NULL);
    while (true) {
        long t = time(NULL);
        double q = random_pulse.get();
        
        hist.fill(q);

        if (t >= t0 + 1.0) {
            std::cout << t << " " << q << std::endl;
            t0 = t;
        
            // Here "update()" is used instead of "append()", to keep only the last objects //
            ds.update(sb::SlowDashDataFrame(schema).tag("hist").time(t) << hist);
        }
    }
    
    return 0;
}
