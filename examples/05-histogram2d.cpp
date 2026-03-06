
#include <iostream>
#include <slowbook.hpp>

namespace sb = slowbook;


int main(void)
{
    sb::Histogram2d hist2d(22, -1.1, 1.1, 22, -1.1, 1.1);

    sb::DataStore_JsonDump ds;
    //sb::DataStore_CsvFile ds("SlowStore_");
    
    sb::SimpleObjectSchema schema("Objects");
    
    for (int i = 0; i < 3; i++) {
        // fill data //
        hist2d.fill(cos(i), sin(i));

        // store data //
        // Here "update()" is used instead of "append()", to keep only the last objects //
        // Without specifying the time with "time(t)", the current time will be used. //
        ds.update(sb::SlowDashDataFrame(schema).tag("hist2d") << hist2d);
    }
    
    return 0;
}
