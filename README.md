# slowbook
C++ library for SlowDash

## 概要
SlowBook は，時系列データやヒストグラムなどを，SlowDash で読める形式でデータベースなどに保存するためのライブラリです．
また，この目的に特化した，超軽量のヒストグラムやグラフなどのデータクラスも提供しています．

Slowbook 自体の外部依存はありません．ただし，使用するデータベースにアクセスするライブラリは必要になります．

Slowbook はヘッダファイルだけで構成されており，Slowbook 自体をコンパイルしたりライブラリをリンクする必要はありません．

## 使用方法
Slowbook のインクルードディレクトリをインクルードパスに追加するだけで使用できます．
```
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
２次元・３次元や誤差つき・誤差なしなどの様々なデータ点型 (`XPoint`) に対する `vector<XPoint>` です．

```
struct Graph: protected std::vector<XPoint> {
    template<typename... Args> void add_point(Args&&... args) {
        std::vector<XPoint>::emplace_back(std::forward<Args>(args)...);
    }
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

例えば，以下のように使います：
```
    // (x,y) と ey を持ったデータ点のグラフを宣言
    slowbook::Graph<slowbook::PointXYEy> graph;
    
    double x=..., y=..., ey=...;
    
    // データ値の組をを仮引数に展開して `add_point()` する
    graph.add_point(x, y, ey);
```

#### トレンド


### データベースインターフェース

