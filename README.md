# AtCoder-heuristic-Contest050
## Contest time 7/6 19:00～7/6 23:00

# 解法

１、簡単な初期解作る(内容理解)
端から置く: 盤面の端に近いマスから順番に岩を置く
理由: 端の方がロボットが滞在する確率が低い（壁にぶつかりやすい）
安全性重視: まずは確実に得点を稼ぐことを優先

得点　22,054,111　(ケース０ 12万点)


２、改良１
内側から外側に動きを変えた(マンハッタン距離で)
得点　51,409,252　　(ケース０ 36万点)

考えるべきこと
・ロボットの生存確率上げるためには？
・てか赤いマス何！(ロボットがもし毎ターンの岩の配置だったらここまで移動できるか？を表す？)

３、改良２ - まばらに配置戦略
思ったこと：中央から外側でスコア2倍以上になったじゃん！これは行けそう。

でも考えてみたら、岩を密集させすぎるとロボットがすぐ止まっちゃうんじゃない？
もっとまばらに配置して、ロボットに「もっと遠くまで行ってもらう」戦略はどうかな。

実装してみた：
- 4x4の格子状に配置（チェスボード模様）
- 中央からの距離は依然として重要？
- 周囲5x5範囲の岩密度をチェックして、密集地帯は避ける
- 基本的には「中央の格子点から順番に」って感じ

狙い：
- ロボットが長距離移動できる空間を確保
- 局所的な「岩の罠」を避ける
- でも中央の危険地帯は早めに処理

次やりたいこと：
- 格子の間隔を調整（4x4 → 6x6とか）
- 壁際の処理をもっと工夫
- シミュレーション回して最適な重み付け見つけたい

得点　　　71,612,598　(ケース０ 46万点)

目標:             145,000,000点


解法4 - バグ修正とパラメータチューニング

**発覚した問題点：**
`calculateSparseScore`の計算式をよく見たら、致命的なバグがあった...。
`densityPenalty`（密度ペナルティ）をスコアに**加算**してた。これだと「周りに岩が多いほど優先して置く」っていう真逆の動きになってた。そりゃスコア伸び悩むわけだ...。

**修正内容：**
1.  **バグ修正:** `return centerDist + sparseBonus + densityPenalty` を `return centerDist + sparseBonus - densityPenalty` に修正。これでやっと「密な場所を避ける」っていう本来の動きになるはず。
2.  **パラメータ調整:**
    *   `gridSize`を4から6に変更。もっと大胆にまばらにしてみる。
    *   `sparseBonus`を-10から-15に強化。格子状に置くメリットをさらに大きくした。
    *   `densityPenalty`の重みを0.5から0.8に。密集地帯へのペナルティを厳しくした。

**狙い：**
- 本来やりたかった「まばら配置」を正しく実装する。
- パラメータをより尖らせることで、戦略の効果を最大化する。

下がった

解法5
基本戦略
ロボットが潰れるリスクを最小化するため、「ロボットが停止する可能性が低いマス」から優先的に岩を置く。

スコアリング関数 (calculateImprovedScore)
各空きマス (r,c) の「危険度」をスコアとして計算し、スコアが低いほど優先する（安全とみなす）。以下の要素を考慮する。

周囲の開放度: マスから移動可能な方向が少ないほどスコア高く（危険）。

隣接する障害物の数: 隣接する岩や盤面の端が多いほどスコア高く（ロボット停止しやすい）。

中央からの距離: 中央から遠いほどスコア少し高く（補助的）。

周囲の初期岩からの距離: 周囲に初期岩が多いほどスコア高く（停止しやすい)


得点　80,427,968(ケース０Score = 562306)


解法６ - 動的貪欲法 + 1手先読み評価

## 基本的な動きの流れ

### 1. **動的な手順決定**
- 従来の解法: 最初に全ての岩の配置順序を決定し、その順番で置き続ける
- 現在の解法: 岩を1つ置くたびに、次に置く岩の場所を再評価して決定

### 2. **具体的な動作手順**
```
1. 現在の盤面で空きマスを全て取得
2. 空きマスを基本評価でソート（危険度の低い順）
3. 上位8候補について「1手先読み評価」を実行
4. 最も評価の良い位置に岩を配置
5. 盤面を更新して1に戻る
```

### 3. **1手先読み評価の仕組み**
各候補位置について以下を計算：
- **直接評価**: その位置に岩を置いた時の基本スコア
- **影響評価**: 岩を置いた後、周囲3マス範囲の次の候補のスコア変化
- **総合評価**: 直接評価 + 影響評価 × 0.1

### 4. **基本評価関数の4要素**
1. **開放度**: 4方向への移動可能距離（重み120倍）
   - 移動方向が少ない = ロボットが停止しやすい = 優先して処理
2. **隣接障害物**: 上下左右の岩・壁の数（重み60倍）
   - 障害物が多い = 停止しやすい = 優先して処理
3. **中央距離**: 中央からの距離（重み0.2倍）
   - 中央ほど危険 = 優先して処理（補助的）
4. **周囲岩**: 3×3範囲の岩の数（重み8倍）
   - 周囲に岩が多い = 停止しやすい = 優先して処理

### 5. **なぜこの動きが効果的か**
- **適応性**: 盤面状況の変化に応じて最適な次の手を選択
- **先読み**: 1手先の影響を考慮することで、局所最適解を回避
- **重み付け**: 「開放度」を最重要視することで、ロボットの実際の移動パターンを正確に予測

### 6. **従来解法との違い**
| 項目 | 従来解法 | 現在解法 |
|------|----------|----------|
| 決定方式 | 静的（一括ソート） | 動的（逐次決定） |
| 評価タイミング | 初期盤面のみ | 毎手更新後 |
| 先読み | なし | 1手先読み |
| 計算量 | 軽い | 重い |
| 適応性 | 低い | 高い |

### 7. **実行時間管理**
- **動的評価フェーズ**: 1.9秒間、時間の許す限り最適な手を選択
- **残り処理フェーズ**: 時間切れ後は従来の一括ソートで残りを処理
- **安全マージン**: 制限時間より100ms短く設定してTLE回避

得点　	100,090,306(ケース０Score = 674918) 

**次の改善案:**
- C++移植による高速化
- 2手先読みの実装
- 評価関数の重み付け調整
- 開放度計算の高精度化


解法７

点数　	100,864,035　Score = 709476)

. クラスタリング戦略
岩を置く場所を「島」のように集中させる
ロボットが入り込めない安全地帯を作る
5. 確率的評価
各マスにロボットが停止する確率を事前計算
モンテカルロシミュレーションで期待値を推定
6. パターン認識
初期の岩配置パターンを認識
パターンに応じて最適戦略を選択
7. 逆算アプローチ
終盤の理想的な盤面状態から逆算
目標状態に向かって岩を配置


解法８

黄金比みたいな置き方にしてみる。

/ 5要素評価関数
score = 開放度×125 + 隣接障害物×55 - 中央距離×2.2 - 黄金比×32 + 周囲岩×2.8

// 3層黄金比評価
1. スパイラル: idealRadius = φ × angle / (2π)
2. グリッド: goldenR1 = n/φ
3. フィボナッチ: {1,1,2,3,5,8,13,21,34}
```
・サイクロイド試したが無理でした……

### 最適化技術
- **動的判断**: 毎手最適解を再計算
- **先読み評価**: 1手先の影響を考慮
- **効率化**: 候補6個に制限、探索範囲2マス

## 総合成果
- **71万点(1.03億)

これまでの問題に比べ全容をつかめなかった。
復習します。
