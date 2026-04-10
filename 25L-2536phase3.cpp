//#include <iostream>
//using namespace std;
//
//const int SIZE = 8;
//char board[SIZE][SIZE];
//
//bool whiteTurn = true;  // White moves first
//int whiteScore = 0, blackScore = 0;
//
//// Initialize chessboard
//void initializeBoard() {
//    string whiteBack = "RNBQKBNR";
//    string blackBack = "rnbqkbnr";
//    for (int c = 0; c < SIZE; c++) {
//        board[0][c] = blackBack[c];
//        board[1][c] = 'p';
//        board[6][c] = 'P';
//        board[7][c] = whiteBack[c];
//    }
//    for (int r = 2; r <= 5; r++)
//        for (int c = 0; c < SIZE; c++)
//            board[r][c] = '.';
//}
//
//// Print the board
//void printBoard() {
//    cout << "\n  a b c d e f g h\n";
//    for (int r = 0; r < SIZE; r++) {
//        cout << 8 - r << " ";
//        for (int c = 0; c < SIZE; c++) {
//            cout << board[r][c] << " ";
//        }
//        cout << 8 - r << "\n";
//    }
//    cout << "  a b c d e f g h\n";
//}
//
//// Convert algebraic notation e2 -> row,column
//bool algebraicToCoords(string s, int& r, int& c) {
//    if (s.size() != 2) return false;
//    c = s[0] - 'a';
//    r = 8 - (s[1] - '0');
//    return r >= 0 && r < SIZE && c >= 0 && c < SIZE;
//}
//
//// Check piece color
//bool isWhite(char p) { return p >= 'A' && p <= 'Z'; }
//bool isBlack(char p) { return p >= 'a' && p <= 'z'; }
//
//// Piece values for scoring
//int pieceValue(char p) {
//    switch (tolower(p)) {
//    case 'p': return 1;
//    case 'n': case 'b': return 3;
//    case 'r': return 5;
//    case 'q': return 9;
//    default: return 0;
//    }
//}
//
//// Simple move legality (geometric only, no castling)
//bool isLegalMove(int sr, int sc, int dr, int dc) {
//    char piece = board[sr][sc];
//    if (piece == '.') return false;
//    bool whitePiece = isWhite(piece);
//    if (whitePiece != whiteTurn) return false; // not your turn
//    char target = board[dr][dc];
//    if (target != '.' && isWhite(target) == whitePiece) return false; // cannot capture own piece
//
//    int dr_abs = dr - sr;
//    int dc_abs = dc - sc;
//
//    switch (tolower(piece)) {
//    case 'p':
//        if (whitePiece && sr == 6 && dr == 4 && dc == sc && board[5][sc] == '.' && board[4][sc] == '.') return true; // 2-step
//        if (whitePiece && dr == sr - 1 && dc == sc && board[dr][dc] == '.') return true; // 1-step
//        if (whitePiece && dr == sr - 1 && (dc == sc + 1 || dc == sc - 1) && isBlack(board[dr][dc])) return true; // capture
//        if (!whitePiece && sr == 1 && dr == 3 && dc == sc && board[2][sc] == '.' && board[3][sc] == '.') return true; // 2-step
//        if (!whitePiece && dr == sr + 1 && dc == sc && board[dr][dc] == '.') return true; // 1-step
//        if (!whitePiece && dr == sr + 1 && (dc == sc + 1 || dc == sc - 1) && isWhite(board[dr][dc])) return true; // capture
//        return false;
//    case 'r': return (sr == dr || sc == dc); // straight lines
//    case 'n': return (abs(dr_abs) == 2 && abs(dc_abs) == 1) || (abs(dr_abs) == 1 && abs(dc_abs) == 2); // L-shape
//    case 'b': return abs(dr_abs) == abs(dc_abs); // diagonal
//    case 'q': return (sr == dr || sc == dc) || (abs(dr_abs) == abs(dc_abs)); // straight or diagonal
//    case 'k': return (abs(dr_abs) <= 1 && abs(dc_abs) <= 1); // 1-step any direction
//    }
//    return false;
//}
//
//// Apply move & scoring
//void applyMove(int sr, int sc, int dr, int dc) {
//    char captured = board[dr][dc];
//    char moving = board[sr][sc];
//    board[dr][dc] = moving;
//    board[sr][sc] = '.';
//
//    if (captured != '.') {
//        if (isWhite(captured)) blackScore += pieceValue(captured);
//        else whiteScore += pieceValue(captured);
//    }
//
//    // Pawn promotion (automatic to queen)
//    if (moving == 'P' && dr == 0) board[dr][dc] = 'Q';
//    if (moving == 'p' && dr == 7) board[dr][dc] = 'q';
//}
//
//// Check if square is attacked by enemy
//bool squareAttacked(int r, int c, bool byWhite) {
//    for (int sr = 0; sr < SIZE; sr++)
//        for (int sc = 0; sc < SIZE; sc++) {
//            char p = board[sr][sc];
//            if (p == '.') continue;
//            if (byWhite && !isWhite(p)) continue;
//            if (!byWhite && !isBlack(p)) continue;
//            bool turnBackup = whiteTurn;
//            whiteTurn = byWhite;
//            if (isLegalMove(sr, sc, r, c)) {
//                whiteTurn = turnBackup;
//                return true;
//            }
//            whiteTurn = turnBackup;
//        }
//    return false;
//}
//
//// Find king position
//bool findKing(bool white, int& kr, int& kc) {
//    char king = white ? 'K' : 'k';
//    for (int r = 0; r < SIZE; r++)
//        for (int c = 0; c < SIZE; c++)
//            if (board[r][c] == king) { kr = r; kc = c; return true; }
//    return false;
//}
//
//// Is king in check
//bool inCheck(bool white) {
//    int kr, kc;
//    if (!findKing(white, kr, kc)) return false;
//    return squareAttacked(kr, kc, !white);
//}
//
//// Any legal move available for player
//bool hasAnyLegalMove(bool white) {
//    for (int sr = 0; sr < SIZE; sr++)
//        for (int sc = 0; sc < SIZE; sc++) {
//            char p = board[sr][sc];
//            if (p == '.') continue;
//            if (isWhite(p) != white) continue;
//            for (int dr = 0; dr < SIZE; dr++)
//                for (int dc = 0; dc < SIZE; dc++) {
//                    char tmp = board[dr][dc];
//                    char moving = board[sr][sc];
//                    if (!isLegalMove(sr, sc, dr, dc)) continue;
//                    board[dr][dc] = moving;
//                    board[sr][sc] = '.';
//                    bool leavesKingInCheck = inCheck(white);
//                    board[sr][sc] = moving;
//                    board[dr][dc] = tmp;
//                    if (!leavesKingInCheck) return true;
//                }
//        }
//    return false;
//}
//
//// Main game loop
//int main() {
//    initializeBoard();
//    cout << "Simple Chess with Scoring & Check Detection\n";
//    cout << "Enter moves like: e2 e4\nType 'exit' to quit\n";
//
//    while (true) {
//        printBoard();
//        cout << "Score -> White: " << whiteScore << "  Black: " << blackScore << "\n";
//        cout << (whiteTurn ? "White" : "Black") << "'s move: ";
//
//        string from, to;
//        cin >> from;
//        if (from == "exit") break;
//        cin >> to;
//
//        int sr, sc, dr, dc;
//        if (!algebraicToCoords(from, sr, sc) || !algebraicToCoords(to, dr, dc)) {
//            cout << "Invalid input!\n"; continue;
//        }
//        if (!isLegalMove(sr, sc, dr, dc)) {
//            cout << "Illegal move!\n"; continue;
//        }
//        applyMove(sr, sc, dr, dc);
//        whiteTurn = !whiteTurn;
//
//        bool oppWhite = !whiteTurn;
//        bool oppInCheck = inCheck(oppWhite);
//        bool oppHasMove = hasAnyLegalMove(oppWhite);
//
//        if (oppInCheck && !oppHasMove) {
//            printBoard();
//            cout << "CHECKMATE! " << (whiteTurn ? "White" : "Black") << " wins.\n";
//            cout << "Final Score -> White: " << whiteScore << "  Black: " << blackScore << "\n";
//            break;
//        }
//        else if (!oppInCheck && !oppHasMove) {
//            printBoard();
//            cout << "STALEMATE! Draw.\n";
//            cout << "Final Score -> White: " << whiteScore << "  Black: " << blackScore << "\n";
//            break;
//        }
//        else if (oppInCheck) {
//            cout << "CHECK on " << (oppWhite ? "White" : "Black") << "!\n";
//        }
//    }
//
//    cout << "Game ended.\n";
//    return 0;
//}
//
////#include <iostream>
////#include <string>
////#include <cctype>
////using namespace std;
////
////const int SIZE = 8;
////char board[SIZE][SIZE];
////bool whiteTurn = true;
////
////// Prototypes
////void initializeBoard();
////void printBoard();
////bool isValidMove(int sx, int sy, int dx, int dy);
////bool isValidPawnMove(int sx, int sy, int dx, int dy);
////bool isValidKnightMove(int sx, int sy, int dx, int dy);
////bool isValidRookMove(int sx, int sy, int dx, int dy);
////bool isValidBishopMove(int sx, int sy, int dx, int dy);
////bool isValidQueenMove(int sx, int sy, int dx, int dy);
////bool isValidKingMove(int sx, int sy, int dx, int dy);
////void makeMove(int sx, int sy, int dx, int dy);
////string getHint(int sx, int sy, int dx, int dy);
////
////// Algebraic "e2" → (row,col)
////bool algebraicToCoords(const string& alg, int& row, int& col) {
////    if (alg.length() != 2) return false;
////
////    char file = tolower(alg[0]);
////    char rank = alg[1];
////
////    if (file < 'a' || file > 'h') return false;
////    if (rank < '1' || rank > '8') return false;
////
////    col = file - 'a';
////    row = 8 - (rank - '0');
////    return true;
////}
////
////void initializeBoard() {
////    string backWhite = "RNBQKBNR";
////    string backBlack = "rnbqkbnr";
////
////    for (int c = 0; c < SIZE; c++) board[0][c] = backBlack[c];
////    for (int c = 0; c < SIZE; c++) board[1][c] = 'p';
////
////    for (int r = 2; r <= 5; r++)
////        for (int c = 0; c < SIZE; c++)
////            board[r][c] = '.';
////
////    for (int c = 0; c < SIZE; c++) board[6][c] = 'P';
////    for (int c = 0; c < SIZE; c++) board[7][c] = backWhite[c];
////}
////
////void printBoard() {
////    cout << "\n   a b c d e f g h\n";
////    for (int r = 0; r < SIZE; r++) {
////        cout << 8 - r << "  ";
////        for (int c = 0; c < SIZE; c++) {
////            cout << board[r][c] << " ";
////        }
////        cout << " " << 8 - r << "\n";
////    }
////    cout << "   a b c d e f g h\n\n";
////}
////
////// ========== MOVEMENT RULES ==========
////
////bool isValidPawnMove(int sx, int sy, int dx, int dy)
////{
////    char p = board[sx][sy];
////    bool white = isupper(p);
////
////    int direction = white ? -1 : +1;
////
////    // Simple forward
////    if (dy == sy && dx == sx + direction && board[dx][dy] == '.') return true;
////
////    // Double forward
////    if (white && sx == 6 && dx == 4 && dy == sy && board[5][sy] == '.' && board[4][sy] == '.') return true;
////    if (!white && sx == 1 && dx == 3 && dy == sy && board[2][sy] == '.' && board[3][sy] == '.') return true;
////
////    // Diagonal capture
////    if (dx == sx + direction && (dy == sy + 1 || dy == sy - 1)) {
////        if (board[dx][dy] != '.' && (isupper(board[dx][dy]) != white))
////            return true;
////    }
////
////    return false;
////}
////
////bool isValidKnightMove(int sx, int sy, int dx, int dy) {
////    int dr = abs(dx - sx);
////    int dc = abs(dy - sy);
////    if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2))) return false;
////
////    char dest = board[dx][dy];
////    if (dest == '.') return true;
////    return isupper(dest) != isupper(board[sx][sy]);
////}
////
////bool isValidRookMove(int sx, int sy, int dx, int dy) {
////    if (sx != dx && sy != dy) return false;
////
////    int stepR = (dx > sx) ? 1 : (dx < sx ? -1 : 0);
////    int stepC = (dy > sy) ? 1 : (dy < sy ? -1 : 0);
////
////    int r = sx + stepR, c = sy + stepC;
////    while (r != dx || c != dy) {
////        if (board[r][c] != '.') return false;
////        r += stepR; c += stepC;
////    }
////
////    if (board[dx][dy] == '.') return true;
////    return isupper(board[dx][dy]) != isupper(board[sx][sy]);
////}
////
////bool isValidBishopMove(int sx, int sy, int dx, int dy) {
////    if (abs(dx - sx) != abs(dy - sy)) return false;
////
////    int stepR = (dx > sx) ? 1 : -1;
////    int stepC = (dy > sy) ? 1 : -1;
////
////    int r = sx + stepR, c = sy + stepC;
////    while (r != dx && c != dy) {
////        if (board[r][c] != '.') return false;
////        r += stepR; c += stepC;
////    }
////
////    if (board[dx][dy] == '.') return true;
////    return isupper(board[dx][dy]) != isupper(board[sx][sy]);
////}
////
////bool isValidQueenMove(int sx, int sy, int dx, int dy) {
////    return isValidRookMove(sx, sy, dx, dy) || isValidBishopMove(sx, sy, dx, dy);
////}
////
////bool isValidKingMove(int sx, int sy, int dx, int dy) {
////    if (abs(dx - sx) <= 1 && abs(dy - sy) <= 1) {
////        if (board[dx][dy] == '.') return true;
////        return isupper(board[dx][dy]) != isupper(board[sx][sy]);
////    }
////    return false;
////}
////
////// ========== HINT SYSTEM ==========
////
////string getHint(int sx, int sy, int dx, int dy) {
////    char piece = board[sx][sy];
////    if (piece == '.') return "There is no piece on the source square.";
////
////    char P = tolower(piece);
////
////    // Pawn hints
////    if (P == 'p') {
////        if (sy == dy && board[dx][dy] != '.') return "Pawn cannot move forward into an occupied square.";
////        if (sy != dy && board[dx][dy] == '.') return "Pawn cannot move diagonally unless capturing.";
////        return "Invalid pawn movement.";
////    }
////
////    // Knight
////    if (P == 'n') return "Knight moves in an L-shape (2+1).";
////
////    // Rook
////    if (P == 'r') {
////        if (sx != dx && sy != dy) return "Rook moves only in straight lines.";
////        return "Rook cannot jump over pieces.";
////    }
////
////    // Bishop
////    if (P == 'b') {
////        if (abs(dx - sx) != abs(dy - sy)) return "Bishop moves diagonally.";
////        return "Bishop path is blocked.";
////    }
////
////    // Queen
////    if (P == 'q') return "Queen moves like rook or bishop (straight or diagonal).";
////
////    // King
////    if (P == 'k') return "King moves 1 square in any direction.";
////
////    return "Invalid move.";
////}
////
////// ========== MOVE SYSTEM ==========
////
////bool isValidMove(int sx, int sy, int dx, int dy) {
////    char piece = tolower(board[sx][sy]);
////
////    switch (piece) {
////    case 'p': return isValidPawnMove(sx, sy, dx, dy);
////    case 'n': return isValidKnightMove(sx, sy, dx, dy);
////    case 'r': return isValidRookMove(sx, sy, dx, dy);
////    case 'b': return isValidBishopMove(sx, sy, dx, dy);
////    case 'q': return isValidQueenMove(sx, sy, dx, dy);
////    case 'k': return isValidKingMove(sx, sy, dx, dy);
////    }
////    return false;
////}
////
////void makeMove(int sx, int sy, int dx, int dy) {
////    board[dx][dy] = board[sx][sy];
////    board[sx][sy] = '.';
////}
////
////// ========== MAIN LOOP ==========
////
////int main() {
////    initializeBoard();
////
////    while (true) {
////        printBoard();
////        cout << (whiteTurn ? "White" : "Black") << "'s move: ";
////
////        string from, to;
////        cin >> from >> to;
////        if (from == "exit") break;
////
////        int fr, fc, tr, tc;
////        if (!algebraicToCoords(from, fr, fc) ||
////            !algebraicToCoords(to, tr, tc)) {
////            cout << "Invalid format. Use e.g. e2 e4\n";
////            continue;
////        }
////
////        if (isupper(board[fr][fc]) != whiteTurn) {
////            cout << "❌ Not your turn.\n";
////            continue;
////        }
////
////        if (isValidMove(fr, fc, tr, tc)) {
////            makeMove(fr, fc, tr, tc);
////            whiteTurn = !whiteTurn;
////        }
////        else {
////            cout << "❌ Invalid move! " << getHint(fr, fc, tr, tc) << "\n";
////        }
////    }
////
////    return 0;
////}
//
