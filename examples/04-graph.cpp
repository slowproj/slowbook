
#include <iostream>
#include <slowbook.hpp>

namespace sb = slowbook;


int main(void)
{
    sb::Graph<sb::PointXYEy> graph;

    sb::DataStore_CsvFile ds("SlowStore_");
    //sb::DataStore_JsonDump ds;
    
    sb::SimpleObjectSchema schema("Objects");
    
    for (int i = 0; i < 3; i++) {
        // fill data (x, y, ey) //
        graph.add_point(i, i*i, 1);

        // store data //
        // Here "update()" is used instead of "append()", to keep only the last objects //
        // Without specifying the time with "time(t)", the current time will be used. //
        ds.update(sb::SlowDashDataFrame(schema).tag("graph") << graph);
    }
    
    return 0;
}
