# Slowbook

## Overview
Slowbook is a C++ library for storing time-series data and objects such as histograms in formats readable by SlowDash.
It also provides ultra-lightweight data classes for histograms, graphs, and more, optimized for online data acquisition and storage.

Slowbook itself has no external dependencies. However, you still need separate libraries to access whichever database backend you use.

Slowbook is header-only, so you do not need to compile Slowbook itself or link against a Slowbook binary library.

## Usage
You can use it simply by adding Slowbook's include directory to your include path.
```Makefile
CXXFLAGS=-std=c++17 -I$(SLOWBOOK_DIR)/includes
```

See the examples under `examples` for reference.


## Contents

### Data classes
Slowbook provides the following data classes:
- Histogram, 2D histogram
- Graphs (2D/3D, with/without errors)
- Trends (reduced data per time bin)

#### Histogram
It is essentially a `vector<double>` with binning support via `fill(double value)`:

```c++
struct Histogram: protected std::vector<double> {
    double min, max;
    double overflow, underflow;
    using Base = std::vector<double>;
    using Base::operator[];
    using Base::size;
    using Base::empty;
    using Base::begin;
    using Base::end;
    using Base::cbegin;
    using Base::cend;
  public:
    Histogram(unsigned nbins, double min, double max);
    void fill(double value, double weight=1);
    void clear(void);  // also clears overflow/underflow
```

#### Graph
It is essentially a `vector<XPoint>`, where `XPoint` can represent various point types (2D/3D, with/without errors, etc.):

```c++
template<class XPoint>
struct Graph: protected std::vector<XPoint> {
    using Base = std::vector<XPoint>;
    using Base::operator[];
    using Base::clear;
    using Base::size;
    using Base::empty;
    using Base::begin;
    using Base::end;
    using Base::cbegin;
    using Base::cend;
  public:
    template<typename... XArgs> void add_point(XArgs&&... args);
```

For `XPoint`, the following predefined types are available (excerpt):
- `PointXY`: 2D point (x, y) (basically `array<double,2>`)
- `PointXYZ`: 3D point (x, y, z) (basically `array<double,3>`)
- `PointXYEy`: 2D point (x, y) with error `ey` on y (basically `array<double,3>`)
- `PointXYExEy`: 2D point (x, y) with errors `ex`, `ey` (basically `array<double,4>`)

For example:
```c++
    // Declare a graph of points with (x, y) and ey
    slowbook::Graph<slowbook::PointXYEy> graph;
    
    double x=..., y=..., ey=...;
    
    // Pass data values as variadic arguments to add_point()
    graph.add_point(x, y, ey);
```

#### Trend
A trend is a time series of reduced values (such as averages) for each time bin.
It is essentially a `vector<XTrendPoint>`, where `XTrendPoint` is a reduced-point type.

Use `fill(time, value)` to record data.

For `XTrendPoint`, the following predefined types are available (excerpt):
- `TrendPointMean`: Mean value per time bin
- `TrendPointMeanErr`: Mean value and its error (SEM) per time bin
- `TrendPointMeanMinMax`: Mean, max, and min per time bin
- `TrendPointSum`: Sum per time bin
- `TrendPointN`: Number of entries per time bin

Usage example:
```c++
    slowbook::Trend<slowbook::TrendPointMeanMinMax> trend(TIME_BIN_WIDTH);

    double time = ..., value = ...;

    trend.fill(time, value);
```

A common special case is a rate trend.
It is specialized for event frequency. You record data with `fill(t)` and keep a time series of rates.
Example:

```c++
    slowbook::RateTrend rate_trend(TIME_BIN_WIDTH);

    double event_time = ...;

    rate_trend.fill(event_time);
```


### Database interface

Slowbook can write numeric values or data objects to databases in a SlowDash-readable format, either as full time series or as latest-value snapshots.

#### Numeric values
To store a numeric time series in a CSV file, use the following (`examples/01-numeric.cpp`):
```c++
    slowbook::DataStore_CsvFile ds(DB_URL);
    slowbook::SimpleNumericSchema schema(TABLE_NAME);   // schema to store numeric values directly
    
    while (/* running */ ...) {
        double t = ..., x = ...;
        ds.append(slowbook::SlowDashDataFrame(schema).tag(CHANNEL_NAME).time(t) << x);
    }
```
This generates a file named `{DB_URL}_{TABLE_NAME}.csv` with content like:
```csv
time,channel,value
1772778516,ch0,0.533202
1772778517,ch0,0.195257
1772778518,ch0,-0.332786
...
```

If you use `DataStore::update()` instead of `DataStore::append()`, only the latest value is kept, rather than storing every point in the series.
The exact behavior of `append()` and `update()` depends on the backend. For CSV, only append is available.

#### Data objects
To store histograms or graphs:
```c++
    slowbook::Histogram hist(NBINS, BINMIN, BINMAX);
    
    slowbook::DataStore_CsvFile ds(DB_URL);
    slowbook::SimpleObjectSchema schema(TABLE_NAME);   // schema to store objects as JSON strings

    while (/* running */ ...) {
        for (/* received data points */ ...) {
            double x = ...;
            hist.fill(x);
        }

        ds.update(slowbook::SlowDashDataFrame(schema).tag(HISTGRAM_NAME) << hist);
    }
```
Unlike numeric time-series data, you usually do not need to write every time a new point arrives.

This generates a file named `{DB_URL}_{TABLE_NAME}.csv` with content like:
```csv
time,channel,value
1772835468,hist01,{ "_attr": { "Underflow": 0\, "Overflow": 0 }\, "bins": { "min": 0\, "max": 100 }\, "counts": [ 1\, 4\, 1\, 1\, 1\, 0\, 1\, 0\, 0\, 0 ] }
1772835469,hist01,{ "_attr": { "Underflow": 0\, "Overflow": 0 }\, "bins": { "min": 0\, "max": 100 }\, "counts": [ 1\, 10\, 4\, 2\, 4\, 0\, 1\, 0\, 0\, 0 ] }
1772835470,hist01,{ "_attr": { "Underflow": 0\, "Overflow": 0 }\, "bins": { "min": 0\, "max": 100 }\, "counts": [ 2\, 12\, 5\, 4\, 4\, 1\, 1\, 0\, 0\, 0 ] }
...
```

#### Trend
Recording trends is slightly different.
Each point in a trend object must be converted so that it is stored as an individual time-series point in the database.
(You can store the trend object itself as a data value, but that is uncommon.)
```c++
    slowbook::RateTrend rate_trend(TIME_BIN_WIDTH);
    
    slowbook::DataStore_CsvFile ds(DB_URL);
    slowbook::SimpleNumericSchema schema(TABLE_NAME);   // use a normal numeric schema

    while (/* running */ ...) {
        for (/* received data points */ ...) {
            double t = ...;
            rate_trend.fill(t);
        }
        
        ds.append(slowbook::SlowDashDataFrame(schema).tag(TREND_NAME).time(t) << rate_trend);
        rate_trend.truncate();  // remove data that has already been written
    }
```
If left as-is, trend time bins will grow indefinitely, so already-written points are removed sequentially.

This generates a file like `SlowStore_NumericData.csv`:
```csv
time,channel,value
1772778833,rate,29.0
1772778834,rate,27.0
1772778835,rate,23.0
```

## Reading with SlowDash
Data written using `slowbook::SimpleXXXSchema` shares the same column names, so it can be read with a `SlowdashProject.yaml` like this (details vary slightly by backend; Redis does not require explicit schema configuration):
```yaml
slowdash_project:
  data_source:
    url: DB_URL
    time_series:
      schema: TABLE_NAME [channel] @time(unix) = value
    object_time_series:
      schema: TABLE_NAME [channel] @time(unix) = value
```

## Internal structure

### Database interface
`DataStore` is the abstract base class for database interfaces:
```c++
class DataStore {
  public:
    void append(const RecordSet& record_set);
    void update(const RecordSet& record_set);
```

Derived classes implement access for each backend. The following are currently available:
- `DataStore_CSV`: writes to CSV files, append-only
- `DataStore_JSON`: outputs JSON format, append-only
- `DataStore_Redis`: writes to Redis key-value entries via `update()`, and Redis TimeSeries via `append()`, using the hiredis library

To avoid unnecessary dependencies on unused database libraries, these implementation classes are not included by `#include <slowbook.hpp>`.
Include only what you need (for Redis, `#include <slowbook/datastore_Redis.hpp>`) and link required libraries in your Makefile / CMakeLists.txt.


### Data structures for writing
`DataStore::append()` / `DataStore::update()` accepts either a `Record` (a single data row/time-series point) or a `RecordSet` (a set of multiple points).
(The formal parameter type is `RecordSet`, but because `RecordSet` has a constructor `RecordSet(const Record&)`, passing a `Record` is converted automatically.)

`Record` corresponds to the SlowDash data model and consists of columns such as:
- Time
- Tags (channel, address, etc.; usually one, but multiple are possible)
- Field values (measured/read values; at least one, possibly multiple)

A typical record containing time `t`, tag `"ch01"`, and field value `x` is built as:
```c++
Record record(schema);
record.time(t).tag("ch01").value(x);
```

Here, `schema` is an instance of `Schema`, which describes the record's column structure.
The common `(time:UNIX, channel:string, field:double)` layout for time-series data is provided as predefined `SimpleNumericSchema`.
Instead of using `SimpleNumericSchema`, you can define the same schema manually:
```c++
    Schema schema(TABLE_NAME);
    schema.add_time<long>("time");
    schema.add_tag<std::string>("channel");
    schema.add_field<double>("value");
```

There is also `SimpleObjectSchema` for storing JSON-serialized histograms or graphs, equivalent to:
```c++
    Schema schema(TABLE_NAME);
    schema.add_time<long>("time");
    schema.add_tag<std::string>("channel");
    schema.add_field<std::string>("value");
```

Because `Record` is an ephemeral object that holds only one row, a typical style is:
```c++
data_store.append(Record(numeric_schema).time(time).tag(channel).value(numeric_data));
```

When writing a histogram or graph as a record field value, it must be serialized to a JSON string.
To serialize Slowbook data class instances into the SlowDash format, use the overloaded function `std::string enslow(const DataClass&)`.

```c++
data_store.append(Record(object_schema).time(time).tag(channel).value(enslow(histogram)));
```

As another way to do the same thing, you can use `SlowDashDataFrame`, a `RecordSet`-derived class specialized for SlowDash format:
```c++
data_store.append(SlowDashDataFrame(numeric_schema).time(time).tag(channel) << numeric_data);
```
```c++
data_store.append(SlowDashDataFrame(object_schema).time(time).tag(channel) << histogram);
```
Here, `operator<<` is overloaded with `SlowDashDataFrame` on the left-hand side, and internally calls `enslow(rhs)`.
At this stage there is little difference from calling `enslow()` yourself for format-independent records, but this notation becomes more uniform when recording trends.

`RecordSet` is essentially `vector<Record>`.
Performance can improve when `DataStore` writes multiple records at once. (The primary purpose of `RecordSet`, however, is to support trends as shown below.)

A trend holds multiple time-series points, so when serialized, it becomes multiple `Record`s, i.e. a `RecordSet`.
For this reason, you cannot directly apply `enslow()` to a trend object.
(If you do, the trend object is serialized as a single graph object. You can store it like a graph, but in most cases that is not the intended behavior.)

```c++
// Warning: this is usually not the desired behavior (records a trend as a graph)
data_store.append(Record(object_schema).time(time).tag(channel).value(enslow(trend)));
```

In the `SlowDashDataFrame` style, `operator<<` is overloaded with `SlowDashDataFrame` (a specialized `RecordSet` derivative) on the left-hand side, so a trend object can be expanded and recorded as multiple time-series points.

```c++
data_store.append(SlowDashDataFrame(numeric_schema).time(time).tag(channel) << trend);
```
Note that the schema used here should be a normal numeric time-series schema, not an object time-series schema.

In this way, the DataFrame style provides a unified form independent of the type of data being recorded.
