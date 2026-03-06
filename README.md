# slowbook
C++ library for SlowDash

## 概要
Slowbook は，時系列データやヒストグラムなどを，SlowDash で読める形式でデータベースなどに保存するための C++ ライブラリです．
また，この目的に特化した，超軽量のヒストグラムやグラフなどのデータクラスも提供しています．

Slowbook 自体の外部依存はありません．ただし，使用するデータベースにアクセスするライブラリは必要になります．

Slowbook はヘッダファイルだけで構成されており，Slowbook 自体をコンパイルしたりライブラリをリンクしたりする必要はありません．

## 使用方法
Slowbook のインクルードディレクトリをインクルードパスに追加するだけで使用できます．
```Makefile
CXXFLAGS=-std=c++17 -I$(SLOWBOOK_DIR)/includes
```

`examples` 以下に例があるので参照してください．


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
struct Graph: protected std::vector<XPoint> {
    template<typename... XArgs> void add_point(XArgs&&... args);
    using std::vector<XPoint>::operator[];
    using std::vector<XPoint>::clear;
    using std::vector<XPoint>::size;
    using std::vector<XPoint>::empty;
    using std::vector<XPoint>::begin;
    using std::vector<XPoint>::end;
```

`XPoint` については，以下のものが事前定義されています（一部抜粋）：
- `PointXY`: (x, y) の２次元のデータ点
- `PointXYZ`: (x, y, z) の３次元のデータ点
- `PointXYEy`: (x, y) の２次元のデータ点と，y に対する誤差 ey
- `PointXYExEy`: (x, y) の２次元のデータ点と，それらに対する誤差 ex, ey

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

実数値の時系列を CSV ファイルに記録する場合は，以下のようにします．(`examples/01-numeric.cpp`)
```c++
    sb::DataStore_CsvFile ds("SlowStore_");
    sb::SimpleNumericSchema schema("NumericData");   // 数値をそのままデータ値として保存するためのスキーマ
    
    while (...) {
        double t = ..., x = ...;
        ds.append(sb::SlowDashDataFrame(schema).tag("ch0").time(t) << x);
    }
```

ヒストグラムやグラフを保存する場合は以下のようにします．(`examples/04-graph.cpp`)
```c++
    sb::Graph<sb::PointXYEy> graph;
    
    sb::DataStore_CsvFile ds("SlowStore_");
    sb::SimpleObjectSchema schema("Objects");   // オブジェクトを JSON 文字列として保存するためのスキーマ

    while (/* running */ ...) {
        for (/* received data points */ ...) {
            double x = ..., y = ..., ey = ...;
            graph.add_point(x, y, ey);
        }

        ds.update(sb::SlowDashDataFrame(schema).tag("graph01") << graph);
    }
```
数値を記録する場合と違って，新しいデータ点が増えるたびに記録する必要はないことに注意してください．

トレンドの記録はちょっと特殊です．
これは，一つのトレンドオブジェクト中の各データ点について，それぞれがデータベースに保存されるれる時系列の点となるように変換する必要があるためです．
（トレンドオブジェクト自体をデータ値として保存することもできますが，普通はやらないです．）
```c++
    sb::RateTrend rate_trend(time_bin_width);
    
    sb::DataStore_CsvFile ds("SlowStore_");
    sb::SimpleNumericSchema schema("NumericData");   // 通常の数値データ用のスキーマを使う

    while (/* running */ ...) {
        for (/* received data points */ ...) {
            double t = ...;
            rate_trend.fill(t);
        }
        
        ds.append(sb::SlowDashDataFrame(schema).tag("rate").time(t) << rate_trend);
        rate_trend.truncate();  // 記録済みのデータは削除
    }
```
トレンドは，何もしないと無限に時刻ビンが増えていってしまうので，記録したデータは順次削除しています．
