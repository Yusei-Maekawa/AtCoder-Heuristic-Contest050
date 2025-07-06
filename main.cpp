#include <bits/stdc++.h>
using namespace std;

#define _USE_MATH_DEFINES
#include <cmath>

bool DEBUG = false;
const long TIME_LIMIT = 1800; // Javaと同じ
long startTime;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

long getCurrentTime() {
    return chrono::duration_cast<chrono::milliseconds>(
        chrono::steady_clock::now().time_since_epoch()
    ).count();
}

// 関数宣言
double calculateGoldenGridBonus(int r, int c, int n, double phi);
double calculateFibonacciBonus(int r, int c, int n);
double calculateGoldenRatioBonus(int r, int c, int n);
double calculateImprovedScore(int r, int c, int n, vector<vector<char>>& currentBoard);
double evaluateWithLookahead(pair<int, int> move, vector<vector<char>>& board, int n);
pair<int, int> selectBestMove(vector<pair<int, int>>& candidates, 
                              vector<vector<char>>& board, int n);

void debug(string msg) {
    if (DEBUG) {
        cerr << "[DEBUG] " << msg << endl;
    }
}

int main() {
    startTime = getCurrentTime();
    
    int n, m;
    cin >> n >> m;
    
    // Java版と同じ：char配列に変更（高速化）
    vector<vector<char>> board(n, vector<char>(n));
    for (int i = 0; i < n; i++) {
        string row;
        cin >> row;
        for (int j = 0; j < n; j++) {
            board[i][j] = row[j];
        }
    }
    
    // 動的貪欲法：盤面状況に応じて逐次決定
    vector<pair<int, int>> result;
    
    while (getCurrentTime() - startTime < TIME_LIMIT) {
        // 現在の空きマスを取得
        vector<pair<int, int>> emptySpaces;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (board[i][j] == '.') {
                    emptySpaces.push_back({i, j});
                }
            }
        }
        
        if (emptySpaces.empty()) break;
        
        // 複数候補から最良を選択（先読み付き）
        pair<int, int> bestMove = selectBestMove(emptySpaces, board, n);
        result.push_back(bestMove);
        
        // 盤面を更新
        board[bestMove.first][bestMove.second] = '#';
    }
    
    // 残りの空きマスを従来の方法で処理
    vector<pair<int, int>> remaining;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (board[i][j] == '.') {
                remaining.push_back({i, j});
            }
        }
    }
    
    sort(remaining.begin(), remaining.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
        double scoreA = calculateImprovedScore(a.first, a.second, n, board);
        double scoreB = calculateImprovedScore(b.first, b.second, n, board);
        return scoreA < scoreB;
    });
    
    for (auto& pos : remaining) {
        result.push_back(pos);
    }
    
    for (auto& pos : result) {
        cout << pos.first << " " << pos.second << "\n";
    }
    
    return 0;
}

// 先読み付き最良手選択（Java版最新版と完全一致）
pair<int, int> selectBestMove(vector<pair<int, int>>& candidates, 
                              vector<vector<char>>& board, int n) {
    pair<int, int> bestMove = {-1, -1};
    double bestScore = DBL_MAX;
    
    // 上位候補数を微調整
    int maxCandidates = min(6, (int)candidates.size()); // 5 -> 6に微調整
    
    // 安定したソート
    sort(candidates.begin(), candidates.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
        double scoreA = calculateImprovedScore(a.first, a.second, n, board);
        double scoreB = calculateImprovedScore(b.first, b.second, n, board);
        if (abs(scoreA - scoreB) < 1e-9) { // 同じスコアの場合
            int posA = a.first * n + a.second;
            int posB = b.first * n + b.second;
            return posA < posB;
        }
        return scoreA < scoreB;
    });
    
    for (int i = 0; i < maxCandidates; i++) {
        pair<int, int> candidate = candidates[i];
        double score = evaluateWithLookahead(candidate, board, n);
        
        if (score < bestScore) {
            bestScore = score;
            bestMove = candidate;
        }
    }
    
    return bestMove.first != -1 ? bestMove : candidates[0];
}

// 1手先読み評価（Java版最新版と完全一致）
double evaluateWithLookahead(pair<int, int> move, vector<vector<char>>& board, int n) {
    // 仮想的に岩を置いた盤面を作成
    vector<vector<char>> tempBoard = board;
    tempBoard[move.first][move.second] = '#';
    
    // この手の直接評価
    double directScore = calculateImprovedScore(move.first, move.second, n, board);
    
    // 次の手の選択肢への影響を評価（範囲を限定）
    vector<pair<int, int>> nextMoves;
    int centerR = move.first;
    int centerC = move.second;
    int searchRange = 2; // 3 -> 2に削減（置いた岩の周囲2マスのみチェック）
    
    for (int i = max(0, centerR - searchRange); i <= min(n-1, centerR + searchRange); i++) {
        for (int j = max(0, centerC - searchRange); j <= min(n-1, centerC + searchRange); j++) {
            if (tempBoard[i][j] == '.') {
                nextMoves.push_back({i, j});
            }
        }
    }
    
    if (nextMoves.empty()) return directScore;
    
    // 次の手の候補の平均スコア（影響度）
    double avgNextScore = 0.0;
    int sampleSize = min(2, (int)nextMoves.size()); // 3 -> 2に削減
    for (int i = 0; i < sampleSize; i++) {
        pair<int, int> nextMove = nextMoves[i];
        avgNextScore += calculateImprovedScore(nextMove.first, nextMove.second, n, tempBoard);
    }
    avgNextScore /= sampleSize;
    
    // 組み合わせ評価
    return directScore + avgNextScore * 0.05; // 0.1 -> 0.05に削減（計算簡略化）
}

// 黄金比・フィボナッチ・スパイラルパターンの評価関数（Java版完全一致）
double calculateGoldenRatioBonus(int r, int c, int n) {
    double bonus = 0.0;
    double phi = 1.618033988749; // 黄金比
    double center = (n - 1) / 2.0;
    
    // 1. 黄金比スパイラル配置
    double angle = atan2(r - center, c - center);
    double distance = sqrt((r - center) * (r - center) + (c - center) * (c - center));
    
    // 黄金比スパイラルの理想的な半径
    double idealRadius = phi * angle / (2 * M_PI);
    if (idealRadius > 0) {
        double radiusDiff = abs(distance - idealRadius);
        if (radiusDiff < 2.0) {
            bonus += 3.0 - radiusDiff; // スパイラルに近いほど高ボーナス
        }
    }
    
    // 2. 黄金比グリッド配置
    double goldenGridBonus = calculateGoldenGridBonus(r, c, n, phi);
    bonus += goldenGridBonus;
    
    // 3. フィボナッチ数列配置
    double fibonacciBonus = calculateFibonacciBonus(r, c, n);
    bonus += fibonacciBonus;
    
    return bonus;
}

// 黄金比グリッド配置ボーナス
double calculateGoldenGridBonus(int r, int c, int n, double phi) {
    double bonus = 0.0;
    
    // 黄金比による分割点
    double goldenR1 = n / phi;
    double goldenR2 = n - goldenR1;
    double goldenC1 = n / phi;
    double goldenC2 = n - goldenC1;
    
    // 黄金比分割点に近いかチェック
    double goldenPoints[] = {goldenR1, goldenR2, goldenC1, goldenC2};
    
    for (double point : goldenPoints) {
        if (abs(r - point) < 1.5) bonus += 2.0;
        if (abs(c - point) < 1.5) bonus += 2.0;
    }
    
    // 黄金比の交点
    if (abs(r - goldenR1) < 1.5 && abs(c - goldenC1) < 1.5) bonus += 3.0;
    if (abs(r - goldenR1) < 1.5 && abs(c - goldenC2) < 1.5) bonus += 3.0;
    if (abs(r - goldenR2) < 1.5 && abs(c - goldenC1) < 1.5) bonus += 3.0;
    if (abs(r - goldenR2) < 1.5 && abs(c - goldenC2) < 1.5) bonus += 3.0;
    
    return bonus;
}

// フィボナッチ数列配置ボーナス
double calculateFibonacciBonus(int r, int c, int n) {
    double bonus = 0.0;
    
    // フィボナッチ数列（最初の数個）
    int fib[] = {1, 1, 2, 3, 5, 8, 13, 21, 34};
    int fibSize = sizeof(fib) / sizeof(fib[0]);
    
    // 座標がフィボナッチ数に近いかチェック
    for (int i = 0; i < fibSize; i++) {
        int f = fib[i];
        if (f >= n) break;
        
        // 行がフィボナッチ数に近い
        if (abs(r - f) <= 1) bonus += 1.0;
        
        // 列がフィボナッチ数に近い
        if (abs(c - f) <= 1) bonus += 1.0;
        
        // 両方がフィボナッチ数に近い
        if (abs(r - f) <= 1 && abs(c - f) <= 1) bonus += 2.0;
    }
    
    return bonus;
}

// 黄金比ベースの微調整版評価関数（Java版完全一致）
double calculateImprovedScore(int r, int c, int n, vector<vector<char>>& currentBoard) {
    double score = 0.0;
    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    // 1. 開放度評価（重み微調整）
    int openDirections = 0;
    for (int i = 0; i < 4; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        while (nr >= 0 && nr < n && nc >= 0 && nc < n && currentBoard[nr][nc] == '.') {
            nr += dr[i];
            nc += dc[i];
        }
        if (nr != r + dr[i] || nc != c + dc[i]) {
            openDirections++;
        }
    }
    score += (4 - openDirections) * 125.0; // 120.0 -> 125.0に微調整

    // 2. 隣接障害物評価（重み微調整）
    int adjacentObstacles = 0;
    for (int i = 0; i < 4; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        if (nr < 0 || nr >= n || nc < 0 || nc >= n) {
            adjacentObstacles++;
        } else if (currentBoard[nr][nc] == '#') {
            adjacentObstacles++;
        }
    }
    score += adjacentObstacles * 55.0; // 60.0 -> 55.0に微調整

    // 3. 中央からの距離（重み微調整）
    double center = (n - 1) / 2.0;
    double centerDist = abs(r - center) + abs(c - center);
    score -= centerDist * 2.2; // 2.0 -> 2.2に微調整

    // 4. 黄金比配置ボーナス（重み微調整）
    double goldenRatioBonus = calculateGoldenRatioBonus(r, c, n);
    score -= goldenRatioBonus * 32.0; // 30.0 -> 32.0に微調整

    // 5. 周囲の初期岩（重み微調整）
    int nearbyInitialRocks = 0;
    for (int dr_ = -1; dr_ <= 1; dr_++) {
        for (int dc_ = -1; dc_ <= 1; dc_++) {
            if (dr_ == 0 && dc_ == 0) continue;
            int nr_ = r + dr_;
            int nc_ = c + dc_;
            if (nr_ >= 0 && nr_ < n && nc_ >= 0 && nc_ < n && currentBoard[nr_][nc_] == '#') {
                nearbyInitialRocks++;
            }
        }
    }
    score += nearbyInitialRocks * 2.8; // 3.0 -> 2.8に微調整

    return score;
}