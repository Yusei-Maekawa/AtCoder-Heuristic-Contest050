import java.util.*;

public class Main {
    static final long TIME_LIMIT = 1800;
    static long startTime;
    static Random rand = new Random();

    public static void main(String[] args) {
        startTime = System.currentTimeMillis();
        try (Scanner sc = new Scanner(System.in)) {
            int n = sc.nextInt();
            int m = sc.nextInt();
            String[][] board = new String[n][n];

            for (int i = 0; i < n; i++) {
                board[i] = sc.next().split("");
            }

            // 動的貪欲法：盤面状況に応じて逐次決定
            List<int[]> result = new ArrayList<>();
            
            while (System.currentTimeMillis() - startTime < TIME_LIMIT) {
                // 現在の空きマスを取得
                List<int[]> emptySpaces = new ArrayList<>();
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        if (board[i][j].equals(".")) {
                            emptySpaces.add(new int[]{i, j});
                        }
                    }
                }
                
                if (emptySpaces.isEmpty()) break;
                
                // 複数候補から最良を選択（先読み付き）
                int[] bestMove = selectBestMove(emptySpaces, board, n);
                result.add(bestMove);
                
                // 盤面を更新
                board[bestMove[0]][bestMove[1]] = "#";
            }

            // 残りの空きマスを従来の方法で処理
            List<int[]> remaining = new ArrayList<>();
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    if (board[i][j].equals(".")) {
                        remaining.add(new int[]{i, j});
                    }
                }
            }
            
            Collections.sort(remaining, (a, b) -> {
                double scoreA = calculateImprovedScore(a[0], a[1], n, board);
                double scoreB = calculateImprovedScore(b[0], b[1], n, board);
                return Double.compare(scoreA, scoreB);
            });
            
            result.addAll(remaining);

            for (int[] pos : result) {
                System.out.println(pos[0] + " " + pos[1]);
            }
        }
    }

    // 黄金比ベースの微調整版（最小限の変更）
    static double calculateImprovedScore(int r, int c, int n, String[][] currentBoard) {
        double score = 0.0;
        int[] dr = {-1, 1, 0, 0};
        int[] dc = {0, 0, -1, 1};

        // 1. 開放度評価（重み微調整）
        int openDirections = 0;
        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i];
            int nc = c + dc[i];
            while (nr >= 0 && nr < n && nc >= 0 && nc < n && currentBoard[nr][nc].equals(".")) {
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
            } else if (currentBoard[nr][nc].equals("#")) {
                adjacentObstacles++;
            }
        }
        score += adjacentObstacles * 55.0; // 60.0 -> 55.0に微調整

        // 3. 中央からの距離（重み微調整）
        double center = (n - 1) / 2.0;
        double centerDist = Math.abs(r - center) + Math.abs(c - center);
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
                if (nr_ >= 0 && nr_ < n && nc_ >= 0 && nc_ < n && currentBoard[nr_][nc_].equals("#")) {
                    nearbyInitialRocks++;
                }
            }
        }
        score += nearbyInitialRocks * 2.8; // 3.0 -> 2.8に微調整

        return score;
    }

    // 先読み評価も微調整
    static int[] selectBestMove(List<int[]> candidates, String[][] board, int n) {
        int[] bestMove = null;
        double bestScore = Double.MAX_VALUE;
        
        // 上位候補数を微調整
        int maxCandidates = Math.min(6, candidates.size()); // 5 -> 6に微調整
        
        // 安定したソート
        Collections.sort(candidates, (a, b) -> {
            double scoreA = calculateImprovedScore(a[0], a[1], n, board);
            double scoreB = calculateImprovedScore(b[0], b[1], n, board);
            int comparison = Double.compare(scoreA, scoreB);
            if (comparison == 0) {
                int posA = a[0] * n + a[1];
                int posB = b[0] * n + b[1];
                return Integer.compare(posA, posB);
            }
            return comparison;
        });
        
        for (int i = 0; i < maxCandidates; i++) {
            int[] candidate = candidates.get(i);
            double score = evaluateWithLookahead(candidate, board, n);
            
            if (score < bestScore) {
                bestScore = score;
                bestMove = candidate;
            }
        }
        
        return bestMove != null ? bestMove : candidates.get(0);
    }

    // 1手先読み評価（高速化版）
    static double evaluateWithLookahead(int[] move, String[][] board, int n) {
        // 仮想的に岩を置いた盤面を作成
        String[][] tempBoard = new String[n][];
        for (int i = 0; i < n; i++) {
            tempBoard[i] = board[i].clone();
        }
        tempBoard[move[0]][move[1]] = "#";
        
        // この手の直接評価
        double directScore = calculateImprovedScore(move[0], move[1], n, board);
        
        // 次の手の選択肢への影響を評価（範囲を限定）
        List<int[]> nextMoves = new ArrayList<>();
        int centerR = move[0];
        int centerC = move[1];
        int searchRange = 2; // 3 -> 2に削減（置いた岩の周囲2マスのみチェック）
        
        for (int i = Math.max(0, centerR - searchRange); i <= Math.min(n-1, centerR + searchRange); i++) {
            for (int j = Math.max(0, centerC - searchRange); j <= Math.min(n-1, centerC + searchRange); j++) {
                if (tempBoard[i][j].equals(".")) {
                    nextMoves.add(new int[]{i, j});
                }
            }
        }
        
        if (nextMoves.isEmpty()) return directScore;
        
        // 次の手の候補の平均スコア（影響度）
        double avgNextScore = 0.0;
        int sampleSize = Math.min(2, nextMoves.size()); // 3 -> 2に削減
        for (int i = 0; i < sampleSize; i++) {
            int[] nextMove = nextMoves.get(i);
            avgNextScore += calculateImprovedScore(nextMove[0], nextMove[1], n, tempBoard);
        }
        avgNextScore /= sampleSize;
        
        // 組み合わせ評価
        return directScore + avgNextScore * 0.05; // 0.1 -> 0.05に削減（計算簡略化）
    }

    // 中央から外側優先の評価関数に変更
    static double calculateGoldenRatioBonus(int r, int c, int n) {
        double bonus = 0.0;
        double phi = 1.618033988749; // 黄金比
        double center = (n - 1) / 2.0;
        
        // 1. 黄金比スパイラル配置
        double angle = Math.atan2(r - center, c - center);
        double distance = Math.sqrt((r - center) * (r - center) + (c - center) * (c - center));
        
        // 黄金比スパイラルの理想的な半径
        double idealRadius = phi * angle / (2 * Math.PI);
        if (idealRadius > 0) {
            double radiusDiff = Math.abs(distance - idealRadius);
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
    static double calculateGoldenGridBonus(int r, int c, int n, double phi) {
        double bonus = 0.0;
        
        // 黄金比による分割点
        double goldenR1 = n / phi;
        double goldenR2 = n - goldenR1;
        double goldenC1 = n / phi;
        double goldenC2 = n - goldenC1;
        
        // 黄金比分割点に近いかチェック
        double[] goldenPoints = {goldenR1, goldenR2, goldenC1, goldenC2};
        
        for (double point : goldenPoints) {
            if (Math.abs(r - point) < 1.5) bonus += 2.0;
            if (Math.abs(c - point) < 1.5) bonus += 2.0;
        }
        
        // 黄金比の交点
        if (Math.abs(r - goldenR1) < 1.5 && Math.abs(c - goldenC1) < 1.5) bonus += 3.0;
        if (Math.abs(r - goldenR1) < 1.5 && Math.abs(c - goldenC2) < 1.5) bonus += 3.0;
        if (Math.abs(r - goldenR2) < 1.5 && Math.abs(c - goldenC1) < 1.5) bonus += 3.0;
        if (Math.abs(r - goldenR2) < 1.5 && Math.abs(c - goldenC2) < 1.5) bonus += 3.0;
        
        return bonus;
    }

    // フィボナッチ数列配置ボーナス
    static double calculateFibonacciBonus(int r, int c, int n) {
        double bonus = 0.0;
        
        // フィボナッチ数列（最初の数個）
        int[] fib = {1, 1, 2, 3, 5, 8, 13, 21, 34};
        
        // 座標がフィボナッチ数に近いかチェック
        for (int f : fib) {
            if (f >= n) break;
            
            // 行がフィボナッチ数に近い
            if (Math.abs(r - f) <= 1) bonus += 1.0;
            
            // 列がフィボナッチ数に近い
            if (Math.abs(c - f) <= 1) bonus += 1.0;
            
            // 両方がフィボナッチ数に近い
            if (Math.abs(r - f) <= 1 && Math.abs(c - f) <= 1) bonus += 2.0;
        }
        
        return bonus;
    }
}