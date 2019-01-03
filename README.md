# DxLibPp

## 開発状況
このライブラリは現在作成中です。

## DXライブラリのC++ラッパー
C言語用のゲーム開発向けライブラリである [DXライブラリ](http://dxlib.o.oo7.jp/) のC++向けラッパーを提供します。

## 特徴
- オブジェクト指向プログラミング(OOP)
    - 画像やフォント、メディア等をオブジェクトとして提供します。開発者は内部で管理されるハンドル等を意識することなく、多態的な関数呼び出しを行うことができます。

```
// DxLib handle style
int handle = LoadGraph("img.png");
DrawGraph(x, y, handle, TRUE);

// DxLibPp OOP style
Graph g{"img.png"};
g.SetX(x);
g.SetY(y);
g.Draw();
```

- 例外機能によるエラーハンドリング
    - 戻り値ではなくC++の例外機能を使用してエラーハンドリングを行うことができます。正常系と異常系のコードが分離され、可読性や保守性が向上します。また、実装には shared_ptr や unique_ptr 等のスマートポインタが採用されているため、例外が発生した場合でも適切にメモリ解放が行われます。

```
// DxLib error handling style by return value
int handle = LoadGraph("img.png");
if (handle == -1) {
    ...
}

// DxLibPp error handling style by exception
try {
    Graph g{"img.png"};
} catch (std::exception & e) {
    ...
}
```

- Windows API を隠蔽
    - 従来のDXライブラリは、使用に際して Windows API のヘッダファイルをインクルードする必要があります。これは C++ スタイルのプログラミングをする上でいくつかの不都合を発生させます。例えば　Windows API には max, min といった名前のマクロが定義されており、これは std::min std::max 等の標準ライブラリの関数と名前の衝突を引き起こします。その他、Windows API が C 時代の資産ということもあり、関数は全てグローバル名前空間に定義されます。 C++ スタイルの名前空間によって整理された管理を好む開発者にとって、これは好ましくないでしょう。DxLibPp は Windows API によって定義される型やマクロの情報を開発者から完全に隠蔽します。開発者は TRUE, FALSE といった Windows API にて int 型の値として定義されるマクロではなく、bool 型定数である true, false を使用することができます。
