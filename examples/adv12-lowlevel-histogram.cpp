
#include <iostream>
#include <slowbook.hpp>

namespace sb = slowbook;


int main(void)
{
    sb::RandomPulse random_pulse(/*rate_cps*/ 10);
    
    sb::Histogram hist(/*nbins*/ 100, /*min*/ 0, /*max*/ 100);
    
    sb::DataStore_JsonDump ds;
    //sb::DataStore_CsvFile ds("SlowStore_");
    
#if 0
    sb::SimpleObjectSchema schema("Objects");
#else
    // the line above is equivalent to using the plain schema with column definitions below
    sb::Schema schema("Objects");
    schema.add_time<long>("time");
    schema.add_tag<std::string>("channel");
    schema.add_field<std::string>("value");
#endif

    long t0 = time(NULL);
    while (true) {
        long t = time(NULL);
        double q = random_pulse.get();
        
        hist.fill(q);

        if (t >= t0 + 1.0) {
            std::cout << t << " " << q << std::endl;
            t0 = t;
        
            // store data //
            // Here "update()" is used instead of "append()", to keep only the last objects //
            // Without specifying the time with "time(t)", the current time will be used. //
#if 0
            ds.update(sb::SlowDashDataFrame(schema).tag("hist").time(t) << hist);
#else
            // alternative way to store data, using SlowDash serializer explicitly //
            ds.update(sb::Record(schema).tag("hist").time(t).value(sb::enslow(hist)));
#endif
        }
    }
    
    return 0;
}
