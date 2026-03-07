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
基本的には，ビン計算をする `fill(double value)` ができる `vector<double>` です：

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
    void clear(void);  // overflow/underflow もクリアする
```

#### グラフ
基本的には，２次元・３次元や誤差つき・誤差なしなどの様々なデータ点型 (`XPoint`) に対する `vector<XPoint>` です．

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
    slowbook::Trend<slowbook::TrendPointMeanMinMax> trend(TIME_BIN_WIDTH);

    double time = ..., value = ...;

    trend.fill(time, value);
```

トレンドのよく使う特殊な場合として，レートトレンドがあります．
これは，事象発生頻度に特化したトレンドです．`fill(t)` によりデータを記録し，レートの時系列を保持します．
以下のように使います：

```c++
    slowbook::RateTrend rate_trend(TIME_BIN_WIDTH);

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

### データベースインターフェース
DataStore はデータベースインターフェースのための抽象ベースクラスです:
```c++
class DataStore {
  public:
    void append(const RecordSet& record_set);
    void update(const RecordSet& record_set);
```

派生クラスで各データベースアクセスを実装しています．現時点で，以下のものが利用可能です．
- `DataStore_CSV`: CSV ファイルに記録．append() のみ可
- `DataStore_JSON`: JSON 形式で出力．append() のみ可
- `DataSource_Redis`: update() で Redis Key-Value に，append() で Redis Time-Series に記録．hiredis ライブラリを使用

使っていないデータベースライブラリへの不要な依存を避けるために，これらの実装クラスは `#include <slowbook.hpp>` には含まれません．
使うものだけを追加でインクルード (Redis なら `#include <slowbook/datastore_Redis.hpp>`)し，Makefile / CMakeLists.txt 内で必要ライブラリをリンクしてください．


### 書き込み用データ構造
DataStore の `append()`/`update()` に渡すのは，単一データ行を表す Record またはその集合の RecordSet です．
（仮引数は `RecordSet` ですが，`RecordSet` のコンストラクタに `RecordSet(const& Record)` があるので，`Record`を渡してもコンパイラが自動で変換してくれます．）

Record は，SlowDash のデータモデルに対応して，以下のようなカラムから構成されます：
- 時刻
- タグ（チャンネルやアドレスなど；普通は一つだけれど，複数も可能）
- フィールド値（読み出した値；最低一つ，複数可能）

時刻 `t`, タグ `ch01`，フィールドデータ値 `x` を保持する一つのデータレコードは，典型的には以下のように作成されます：
```c++
Record record(schema);
record.time(t).tag("ch01").value(x);
```

ここで，`schema` はレコードのカラム構造を記述する Schema クラスのインスタンスです．
時系列データに典型的な `(time:UNIX, channel:string, field:double)` の構成は定義済みの `SimpleNumericSchema` です．
`SimpleNumericSchema` を使用する代わりに，以下のようにして同じものを自分で作成することもできます：
```c++
    slowbook::Schema schema(TABLE_NAME);
    schema.add_time<long>("time");
    schema.add_tag<std::string>("channel");
    schema.add_field<double>("value");
```

他に，JSON シリアライズしたヒストグラムやグラフを保存するための `SimpleObjectSchema` もあり，以下と同等です：
```c++
    sb::Schema schema(TABE_NAME);
    schema.add_time<long>("time");
    schema.add_tag<std::string>("channel");
    schema.add_field<std::string>("value");
```

Record は単一行のデータを保持するだけの使い捨てオブジェクトなので，典型的には以下のように使用します：
```c++
data_store.append(Record(numeric_schema).time(time).tag(channel).value(numeric_data));
```

ヒストグラムやグラフを Record のフィールド値に書く場合，JSON 文字列にシリアライズする必要があります．
Slowbook のデータクラスのインスタンスを SlowDash 形式にシリアライズするために，多重定義関数 `std::string enslow(const DataClass&)` を使用します．

```c++
data_store.append(Record(object_schema).time(time).tag(channel).value(enslow(histogram)));
```

同じことを行う別の方法として，SlowDash フォーマットに特化した RecordSet の派生クラス `SlowDashDataFrame` を使うこともできます.
```c++
data_store.append(SlowDashDataFrame(numeric_schema).time(time).tag(channel) << numeric_data);
```
```c++
data_store.append(SlowDashDataFrame(object_schema).time(time).tag(channel) << histogram);
```
ここでは，`SlowDashDataFrame` を左辺値にとる`<<` 演算子を多重定義して，その中で `enslow(rhs)` を呼び出しています．
この時点ではフォーマット独立な Record に対して `enslow()` を自分で呼ぶのと大差がありませんが，後でトレンドを記録するときに，こちらの方式の方が統一的な記法になります．

RecordSet は，基本的には `vector<Record>` です．DataStore が複数 Record を同時に書き込むことにより，性能の改善をはかることができますが，第一の目的は，トレンドを扱えるようにすることです．

トレンドは，複数の時系列点を保持するため，シリアライズした場合は複数の Record，すなわち RecordSet となります．
このため，トレンドオブジェクトに対して直接 `enslow()` をすることはできません．
（これをすると，トレンドオブジェクトが一つのグラフオブジェクトとしてシリアライズされます．これをグラフのようにデータベースに記録できますが，ほとんどの場合でこれは期待する動作ではないです．）

```c++
// 警告：これは通常望む動作ではない（トレンドをグラフとして記録）
data_store.append(Record(object_schema).time(time).tag(channel).value(enslow(trend)));
```

SlowDashDataFrame を使用する形式では，DataFrame (RecordSet の特化派生）を左辺値として `<<` 演算子を多重定義しているので，トレンドオブジェクトを複数の時系列点に展開して記録できます．

```c++
data_store.append(SlowDashDataFrame(numeric_schema).time(time).tag(channel) << trend);
```
ここで使用するスキーマはオブジェクト時系列ではなく，通常の数値時系列であることに注意してください．

このように，DataFrame を使用する形式の方が，記録するデータ型によらず同じ形で統一的に記述できます．
