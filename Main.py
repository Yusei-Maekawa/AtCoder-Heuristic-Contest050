import sys
import time
import random
import copy

DEBUG = False
TIME_LIMIT = 1900  # 安全マージン
start_time = time.time() * 1000

def get_current_time():
    return time.time() * 1000

def main():
    global start_time
    start_time = get_current_time()
    
    n, m = map(int, input().split())
    board = []
    
    for i in range(n):
        row = list(input().strip())
        board.append(row)
    
    # 動的貪欲法：盤面状況に応じて逐次決定
    result = []
    
    while get_current_time() - start_time < TIME_LIMIT:
        # 現在の空きマスを取得
        empty_spaces = []
        for i in range(n):
            for j in range(n):
                if board[i][j] == '.':
                    empty_spaces.append([i, j])
        
        if not empty_spaces:
            break
        
        # 複数候補から最良を選択（先読み付き）
        best_move = select_best_move(empty_spaces, board, n)
        result.append(best_move)
        
        # 盤面を更新
        board[best_move[0]][best_move[1]] = '#'
    
    # 残りの空きマスを従来の方法で処理
    remaining = []
    for i in range(n):
        for j in range(n):
            if board[i][j] == '.':
                remaining.append([i, j])
    
    remaining.sort(key=lambda pos: calculate_improved_score(pos[0], pos[1], n, board))
    result.extend(remaining)
    
    for pos in result:
        print(pos[0], pos[1])

def select_best_move(candidates, board, n):
    """先読み付き最良手選択（高速化版）"""
    best_move = None
    best_score = float('inf')
    
    # 上位候補のみを評価（計算時間節約）
    max_candidates = min(8, len(candidates))  # 10 -> 8に削減
    
    # 現在の評価でソート
    candidates.sort(key=lambda pos: calculate_improved_score(pos[0], pos[1], n, board))
    
    for i in range(max_candidates):
        candidate = candidates[i]
        score = evaluate_with_lookahead(candidate, board, n)
        
        if score < best_score:
            best_score = score
            best_move = candidate
    
    return best_move if best_move is not None else candidates[0]

def evaluate_with_lookahead(move, board, n):
    """1手先読み評価（高速化版）"""
    # 仮想的に岩を置いた盤面を作成
    temp_board = copy.deepcopy(board)
    temp_board[move[0]][move[1]] = '#'
    
    # この手の直接評価
    direct_score = calculate_improved_score(move[0], move[1], n, board)
    
    # 次の手の選択肢への影響を評価（範囲を限定）
    next_moves = []
    center_r = move[0]
    center_c = move[1]
    search_range = 3  # 置いた岩の周囲3マスのみチェック
    
    for i in range(max(0, center_r - search_range), min(n, center_r + search_range + 1)):
        for j in range(max(0, center_c - search_range), min(n, center_c + search_range + 1)):
            if temp_board[i][j] == '.':
                next_moves.append([i, j])
    
    if not next_moves:
        return direct_score
    
    # 次の手の候補の平均スコア（影響度）
    avg_next_score = 0.0
    sample_size = min(3, len(next_moves))  # 5 -> 3に削減
    for i in range(sample_size):
        next_move = next_moves[i]
        avg_next_score += calculate_improved_score(next_move[0], next_move[1], n, temp_board)
    avg_next_score /= sample_size
    
    # 組み合わせ評価
    return direct_score + avg_next_score * 0.1  # 先読み影響度は10%

def calculate_improved_score(r, c, n, current_board):
    """既存の評価関数（周囲岩チェック範囲を縮小）"""
    score = 0.0
    dr = [-1, 1, 0, 0]
    dc = [0, 0, -1, 1]
    
    # 1. 開放度評価（重み強化）
    open_directions = 0
    for i in range(4):
        nr = r + dr[i]
        nc = c + dc[i]
        while 0 <= nr < n and 0 <= nc < n and current_board[nr][nc] == '.':
            nr += dr[i]
            nc += dc[i]
        if nr != r + dr[i] or nc != c + dc[i]:
            open_directions += 1
    score += (4 - open_directions) * 120.0  # 100.0 -> 120.0
    
    # 2. 隣接障害物評価（重み強化）
    adjacent_obstacles = 0
    for i in range(4):
        nr = r + dr[i]
        nc = c + dc[i]
        if nr < 0 or nr >= n or nc < 0 or nc >= n:
            adjacent_obstacles += 1
        elif current_board[nr][nc] == '#':
            adjacent_obstacles += 1
    score += adjacent_obstacles * 60.0  # 50.0 -> 60.0
    
    # 3. 中央からの距離
    center = (n - 1) / 2.0
    center_dist = abs(r - center) + abs(c - center)
    score += center_dist * 0.2  # 0.1 -> 0.2
    
    # 4. 周囲の初期岩（範囲を-2~2から-1~1に縮小）
    nearby_initial_rocks = 0
    for dr_ in range(-1, 2):
        for dc_ in range(-1, 2):
            if dr_ == 0 and dc_ == 0:
                continue
            nr_ = r + dr_
            nc_ = c + dc_
            if 0 <= nr_ < n and 0 <= nc_ < n and current_board[nr_][nc_] == '#':
                nearby_initial_rocks += 1
    score += nearby_initial_rocks * 8.0  # 5.0 -> 8.0
    
    return score

def debug(msg):
    if DEBUG:
        print(f"[DEBUG] {msg}", file=sys.stderr)

if __name__ == "__main__":
    main()