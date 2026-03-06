
#include <iostream>
#include <slowbook.hpp>
#include <slowbook/datastore_Redis.hpp>

namespace sb = slowbook;


int main(void)
{
    sb::RandomWalk ch0, ch1;
    
    sb::DataStore_Redis ds("redis://localhost/1");
    sb::SimpleNumericSchema schema("NumericData");
    
    while (true) {
        long t = time(NULL);
        double x0 = ch0.get();
        double x1 = ch1.get();

        sb::SlowDashDataFrame df(schema);
        ds.append(sb::SlowDashDataFrame(schema).tag("ch0").time(t) << x0);
        ds.append(sb::SlowDashDataFrame(schema).tag("ch1").time(t) << x1);

        std::cout << t << "  " << x0 << "  " << x1 << std::endl;
        sleep(1);
    }
    
    return 0;
}
