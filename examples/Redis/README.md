## Slowbook データを Redis に保存

### 内容
- **01-numeric.cpp**: 時系列データを Redis TimeSeries に保存
- **02-histogram.cpp**: ヒストグラムを Redis の Key-Value に保存（上書きモード：最新値のみ保持）
- **03-hist-ratetrend.cpp**: 上記にレートトレンドを追加
- **slowdash.d**: 保存したデータを見るための SlowDash 例

### 動作要件
- C++ 17
- Redis がすでに動いていること
- hiredis ライブラリがインストールされていること

### セットアップ
```bash
cd slowbook/examples/Redis
make
```

Slowbook 自体はヘッダファイルだけのライブラリなので，コンパイルの必要はありません．

### 使い方
**以下の例を実行すると，既存の Redis のデータベース 1 にデータを記録します．**
もし既存の Redis に影響を与えたくない場合，docker などで別の Redis を作ってそれを使うようにポート番号を書き換えてください．

ターミナルを３つ開いて，それぞれ以下のコマンドを実行してください：
##### 時系列データの生成と記録
```
./01-numeric
```

##### イベント型データの生成と，ヒストグラムおよびレートトレンドの作成と記録
```
./03-hist-ratetrend
```

##### SlowDash でデータを見る
```
cd slowdash.d
slowdash --port=18881
```

ウェブブラウザで `http://localhost:18881` にアクセスして，作成済みのレイアウト `test` を開いてください．

<img src="screen-SlowbookRedis.png" width="80%">


### 解説
Slowbook で Redis を使うためには，以下の２つが必要です．
- Redis ライブラリ (hiredis) をリンクする (Makefile)
- Redis にデータを保存するように指定する (C++ コード）


#### Makefile / CMakeLists.txt
通常の方法で hiredis ライブラリをリンクするように設定してください．

Makefile の場合は，以下のように `CXXFLAGS` と `LIBS` に hiredis のインクルードディレクトリとライブラリを追加します：
```Makefile
CXXFLAGS+=$(shell pkg-config --cflags hiredis)
LIBS+=$(shell pkg-config --libs hiredis)
```

### C++ コード
以下に `01-numeric.cxx` の全体を示します．
Redis を使用するのに必要なのは，以下の２点だけです：
- Slowbook の Redis コードをインクルード: `#include <slowbook/datastore_Redis.hpp>`
- データストアに Redis を指定：`sb::DataStore_Redis ds("redis://localhost:6379/1");`

```c++

#include <iostream>
#include <slowbook.hpp>
#include <slowbook/datastore_Redis.hpp>

namespace sb = slowbook;


int main(void)
{
    sb::RandomWalk ch0, ch1;
    
    sb::DataStore_Redis ds("redis://localhost:6379/1");
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
```

Slowbook-Redis では，時系列でないデータ (`append()` ではなく `update()` による記録) は，
Redis の普通の Key-Value に保存します．そのため，最新値だけが保持され，また，時刻情報は失われます．

- TODO: ヒストグラムなどの Attributes 部分に手動で時刻情報を記録することはできます．
- TOOD: ヒストグラムなどを時系列データとして，各時刻点ごとに保存することもできます．
