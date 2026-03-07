# Slowbook
Slowbook is a C++ library to write time-series and histogram/graph data to databases in the SlowDash format. <br>
(For English documentation, see [README_EN.md](README_EN.md))

## 概要
Slowbook は，時系列データやヒストグラムなどを，SlowDash で読める形式でデータベースなどに保存するための C++ ライブラリです．
また，オンラインでの使用を念頭に，作成と保存だけに特化した超軽量のヒストグラムやグラフなどのデータクラスも提供しています．

Slowbook 自体の外部依存はありません．ただし，使用するデータベースにアクセスするライブラリは別に必要になります．

Slowbook はヘッダファイルだけで構成されており，Slowbook 自体をコンパイルしたりライブラリをリンクしたりする必要はありません．

## 使用方法
Slowbook のインクルードディレクトリをインクルードパスに追加するだけで使用できます．
```Makefile
CXXFLAGS=-std=c++17 -I$(SLOWBOOK_DIR)/includes
```

`examples` 以下に例があるので参考にしてください．


## 内容

### データクラス
以下のデータクラスが提供されています：
- ヒストグラム，２次元ヒストグラム
- グラフ（２次元・３次元，誤差つき・誤差なし）
- トレンド（時刻ビンごとのデータ縮約）

#### ヒストグラム
基本的には，ビン計算をする `fill(double value)` ができる `vector<dobule>` です：

```c++
struct Histogram: protected std::vector<double> {
    double min, max;
    double overflow, underflow;
    using std::vector<double>::operator[];
    using std::vector<double>::size;
    using std::vector<double>::empty;
    using std::vector<double>::begin;
    using std::vector<double>::end;
  public:
    Histogram(unsigned nbins, double min, double max);
    void fill(double value, double weight=1);
    void clear(void);
```

#### グラフ
基本的には，２次元・３次元や誤差つき・誤差なしなどの様々なデータ点型 (`XPoint`) に対する `vector<XPoint>` です．

```c++
template<class XPoint>
struct Graph: protected std::vector<XPoint> {
    using std::vector<XPoint>::operator[];
    using std::vector<XPoint>::clear;
    using std::vector<XPoint>::size;
    using std::vector<XPoint>::empty;
    using std::vector<XPoint>::begin;
    using std::vector<XPoint>::end;
  public:
    template<typename... XArgs> void add_point(XArgs&&... args);
```

`XPoint` については，以下のものが事前定義されています（一部抜粋）：
- `PointXY`: (x, y) の２次元のデータ点 (基本的に `array<double,2>`))
- `PointXYZ`: (x, y, z) の３次元のデータ点 (基本的に `array<double,3>`))
- `PointXYEy`: (x, y) の２次元のデータ点と，y に対する誤差 ey (基本的に `array<double,3>`))
- `PointXYExEy`: (x, y) の２次元のデータ点と，それらに対する誤差 ex, ey (基本的に `array<double,4>`))

例えば，以下のように使います：
```c++
    // (x,y) と ey を持ったデータ点のグラフを宣言
    slowbook::Graph<slowbook::PointXYEy> graph;
    
    double x=..., y=..., ey=...;
    
    // データ値の組を可変長引数に展開して `add_point()` する
    graph.add_point(x, y, ey);
```

#### トレンド
トレンドとは，時刻ビンごとの縮約データ（平均とか）の時系列です．
基本的には，縮約したデータ点型(`XTrendPoint`) に対する `vector<XTrendPoint>` です．

`fill(time, value)` によりデータを記録します．

`XTrendPoint` については，以下のものが事前定義されています（一部抜粋）：
- `TrendPointMean`: 時刻ビンごとの平均値
- `TrendPointMeanErr`: 時刻ビンごとの平均値とそのエラー (sem)
- `TrendPointMeanMinMax`: 時刻ビンごとの平均値，最大値と最小値
- `TrendPointSum`: 時刻ビンごとの合計値
- `TrendPointN`: 時刻ビンごとのデータ数

以下のように使います：
```c++
    sb::Trend<sb::TrendPointMeanMinMax> trend(time_bin_width);

    double time = ..., value = ...;

    trend.fill(time, value);
```

トレンドのよく使う特殊な場合として，レートトレンドがあります．
これは，事象発生頻度に特化したトレンドです．`fill(t)` によりデータを記録し，レートの時系列を保持します．
以下のように使います：

```c++
    slowbook::RateTrend rate_trend(time_bin_width);

    double event_time = ...;

    rate_trend.fill(event_time);
```


### データベースインターフェース

Slowbook は，実数値もしくはデータオブジェクトの時系列または即値を SlowDash が読める形式でデータベースに書き込むことができます．

#### 実数値
実数値の時系列を CSV ファイルに記録する場合は，以下のようにします．(`examples/01-numeric.cpp`)
```c++
    slowbook::DataStore_CsvFile ds(DB_URL);
    slowbook::SimpleNumericSchema schema(TABLE_NAME);   // 数値をそのままデータ値として保存するためのスキーマ
    
    while (/* running */ ...) {
        double t = ..., x = ...;
        ds.append(slowbook::SlowDashDataFrame(schema).tag(CHANNEL_NAME).time(t) << x);
    }
```
これにより，以下のような内容の `{DB_URL}_{TABLE_NAME}.csv` というファイルが生成されます：
```csv
time,channel,value
1772778516,ch0,0.533202
1772778517,ch0,0.195257
1772778518,ch0,-0.332786
...
```

`DataStore::append()` の代わりに `DataStore::update()` を使うと，時系列の全ての点を保存する代わりに最新値だけを保存するようにできます．
`append()` や `update()` の実際の振る舞いは，使用するデータベースによります．CSV では append のみ可能です．

#### データオブジェクト
ヒストグラムやグラフを保存する場合は以下のようにします．(`examples/02-histogram.cpp`)
```c++
    slowbook::Histogram hist(NBINS, BINMIN, BINMAX);
    
    slowbook::DataStore_CsvFile ds(DB_URL);
    slowbook::SimpleObjectSchema schema(TABLE_NAME);   // オブジェクトを JSON 文字列として保存するためのスキーマ

    while (/* running */ ...) {
        for (/* received data points */ ...) {
            double x = ...;
            hist.fill(x);
        }

        ds.update(slowbook::SlowDashDataFrame(schema).tag(HISTGRAM_NAME) << hist);
    }
```
数値時系列を記録する場合と違って，通常は新しいデータ点が増えるたびに記録する必要はないことに注意してください．

これにより，以下のような内容の `{DB_URL}_{TABLE_NAME}.csv` というファイルが生成されます：
```csv
time,channel,value
1772835468,hist01,{ "_attr": { "Underflow": 0\, "Overflow": 0 }\, "bins": { "min": 0\, "max": 100 }\, "counts": [ 1\, 4\, 1\, 1\, 1\, 0\, 1\, 0\, 0\, 0 ] }
1772835469,hist01,{ "_attr": { "Underflow": 0\, "Overflow": 0 }\, "bins": { "min": 0\, "max": 100 }\, "counts": [ 1\, 10\, 4\, 2\, 4\, 0\, 1\, 0\, 0\, 0 ] }
1772835470,hist01,{ "_attr": { "Underflow": 0\, "Overflow": 0 }\, "bins": { "min": 0\, "max": 100 }\, "counts": [ 2\, 12\, 5\, 4\, 4\, 1\, 1\, 0\, 0\, 0 ] }
...
```

#### トレンド
トレンドの記録はちょっと特殊です．
これは，一つのトレンドオブジェクト中の各データ点について，それぞれがデータベースに保存されるれる時系列の点となるように変換する必要があるためです．
（トレンドオブジェクト自体をデータ値として保存することもできますが，普通はやらないです．）
```c++
    slowbook::RateTrend rate_trend(TIME_BIN_WIDTH);
    
    slowbook::DataStore_CsvFile ds(DB_URL);
    slowbook::SimpleNumericSchema schema(TABLE_NAME);   // 通常の数値データ用のスキーマを使う

    while (/* running */ ...) {
        for (/* received data points */ ...) {
            double t = ...;
            rate_trend.fill(t);
        }
        
        ds.append(slowbook::SlowDashDataFrame(schema).tag(TREND_NAME).time(t) << rate_trend);
        rate_trend.truncate();  // 記録済みのデータは削除
    }
```
トレンドは，何もしないと無限に時刻ビンが増えていってしまうので，記録したデータは順次削除しています．

これにより，以下のような内容の `SlowStore_NumericData.csv` というファイルが生成されます：
```csv
time,channel,value
1772778833,rate,29.0
1772778834,rate,27.0
1772778835,rate,23.0
```

## SlowDash で読む
`slowbook::SimpleXXXSchema` で記録したデータは，全て同じカラム名なので，以下のような感じの `SlowdashProject.yaml` で読むことができます（データベースごとに多少異なります．Redis の場合は schema 指定はありません)：
```yaml
slowdash_project:
  data_source:
    url: DB_URL
    time_series:
      schema: TABLE_NAME [channel] @time(unix) = value
    object_time_series:
      schema: TABLE_NAME [channel] @time(unix) = value
```

## 内部構造
