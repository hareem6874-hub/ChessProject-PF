//﻿#include <iostream>
//#include <string>
//#include <cctype>
//using namespace std;
//
//const int SIZE = 8;
//char board[SIZE][SIZE];
//bool whiteTurn = true;
//
//// Prototypes
//void initializeBoard();
//void printBoard();
//bool isValidMove(int sx, int sy, int dx, int dy);
//bool isValidPawnMove(int sx, int sy, int dx, int dy);
//bool isValidKnightMove(int sx, int sy, int dx, int dy);
//bool isValidRookMove(int sx, int sy, int dx, int dy);
//bool isValidBishopMove(int sx, int sy, int dx, int dy);
//bool isValidQueenMove(int sx, int sy, int dx, int dy);
//bool isValidKingMove(int sx, int sy, int dx, int dy);
//void makeMove(int sx, int sy, int dx, int dy);
//string getHint(int sx, int sy, int dx, int dy);
//
//// Algebraic "e2" → (row,col)
//bool algebraicToCoords(const string& alg, int& row, int& col) {
//    if (alg.length() != 2) return false;
//
//    char file = tolower(alg[0]);
//    char rank = alg[1];
//
//    if (file < 'a' || file > 'h') return false;
//    if (rank < '1' || rank > '8') return false;
//
//    col = file - 'a';
//    row = 8 - (rank - '0');
//    return true;
//}
//
//void initializeBoard() {
//    string backWhite = "RNBQKBNR";
//    string backBlack = "rnbqkbnr";
//
//    for (int c = 0; c < SIZE; c++) board[0][c] = backBlack[c];
//    for (int c = 0; c < SIZE; c++) board[1][c] = 'p';
//
//    for (int r = 2; r <= 5; r++)
//        for (int c = 0; c < SIZE; c++)
//            board[r][c] = '.';
//
//    for (int c = 0; c < SIZE; c++) board[6][c] = 'P';
//    for (int c = 0; c < SIZE; c++) board[7][c] = backWhite[c];
//}
//
//void printBoard() {
//    cout << "\n   a b c d e f g h\n";
//    for (int r = 0; r < SIZE; r++) {
//        cout << 8 - r << "  ";
//        for (int c = 0; c < SIZE; c++) {
//            cout << board[r][c] << " ";
//        }
//        cout << " " << 8 - r << "\n";
//    }
//    cout << "   a b c d e f g h\n\n";
//}
//
//// ========== MOVEMENT RULES ==========
//
//bool isValidPawnMove(int sx, int sy, int dx, int dy)
//{
//    char p = board[sx][sy];
//    bool white = isupper(p);
//
//    int direction = white ? -1 : +1;
//
//    // Simple forward
//    if (dy == sy && dx == sx + direction && board[dx][dy] == '.') return true;
//
//    // Double forward
//    if (white && sx == 6 && dx == 4 && dy == sy && board[5][sy] == '.' && board[4][sy] == '.') return true;
//    if (!white && sx == 1 && dx == 3 && dy == sy && board[2][sy] == '.' && board[3][sy] == '.') return true;
//
//    // Diagonal capture
//    if (dx == sx + direction && (dy == sy + 1 || dy == sy - 1)) {
//        if (board[dx][dy] != '.' && (isupper(board[dx][dy]) != white))
//            return true;
//    }
//
//    return false;
//}
//
//bool isValidKnightMove(int sx, int sy, int dx, int dy) {
//    int dr = abs(dx - sx);
//    int dc = abs(dy - sy);
//    if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2))) return false;
//
//    char dest = board[dx][dy];
//    if (dest == '.') return true;
//    return isupper(dest) != isupper(board[sx][sy]);
//}
//
//bool isValidRookMove(int sx, int sy, int dx, int dy) {
//    if (sx != dx && sy != dy) return false;
//
//    int stepR = (dx > sx) ? 1 : (dx < sx ? -1 : 0);
//    int stepC = (dy > sy) ? 1 : (dy < sy ? -1 : 0);
//
//    int r = sx + stepR, c = sy + stepC;
//    while (r != dx || c != dy) {
//        if (board[r][c] != '.') return false;
//        r += stepR; c += stepC;
//    }
//
//    if (board[dx][dy] == '.') return true;
//    return isupper(board[dx][dy]) != isupper(board[sx][sy]);
//}
//
//bool isValidBishopMove(int sx, int sy, int dx, int dy) {
//    if (abs(dx - sx) != abs(dy - sy)) return false;
//
//    int stepR = (dx > sx) ? 1 : -1;
//    int stepC = (dy > sy) ? 1 : -1;
//
//    int r = sx + stepR, c = sy + stepC;
//    while (r != dx && c != dy) {
//        if (board[r][c] != '.') return false;
//        r += stepR; c += stepC;
//    }
//
//    if (board[dx][dy] == '.') return true;
//    return isupper(board[dx][dy]) != isupper(board[sx][sy]);
//}
//
//bool isValidQueenMove(int sx, int sy, int dx, int dy) {
//    return isValidRookMove(sx, sy, dx, dy) || isValidBishopMove(sx, sy, dx, dy);
//}
//
//bool isValidKingMove(int sx, int sy, int dx, int dy) {
//    if (abs(dx - sx) <= 1 && abs(dy - sy) <= 1) {
//        if (board[dx][dy] == '.') return true;
//        return isupper(board[dx][dy]) != isupper(board[sx][sy]);
//    }
//    return false;
//}
//
//// ========== HINT SYSTEM ==========
//
//string getHint(int sx, int sy, int dx, int dy) {
//    char piece = board[sx][sy];
//    if (piece == '.') return "There is no piece on the source square.";
//
//    char P = tolower(piece);
//
//    // Pawn hints
//    if (P == 'p') {
//        if (sy == dy && board[dx][dy] != '.') return "Pawn cannot move forward into an occupied square.";
//        if (sy != dy && board[dx][dy] == '.') return "Pawn cannot move diagonally unless capturing.";
//        return "Invalid pawn movement.";
//    }
//
//    // Knight
//    if (P == 'n') return "Knight moves in an L-shape (2+1).";
//
//    // Rook
//    if (P == 'r') {
//        if (sx != dx && sy != dy) return "Rook moves only in straight lines.";
//        return "Rook cannot jump over pieces.";
//    }
//
//    // Bishop
//    if (P == 'b') {
//        if (abs(dx - sx) != abs(dy - sy)) return "Bishop moves diagonally.";
//        return "Bishop path is blocked.";
//    }
//
//    // Queen
//    if (P == 'q') return "Queen moves like rook or bishop (straight or diagonal).";
//
//    // King
//    if (P == 'k') return "King moves 1 square in any direction.";
//
//    return "Invalid move.";
//}
//
//// ========== MOVE SYSTEM ==========
//
//bool isValidMove(int sx, int sy, int dx, int dy) {
//    char piece = tolower(board[sx][sy]);
//
//    switch (piece) {
//    case 'p': return isValidPawnMove(sx, sy, dx, dy);
//    case 'n': return isValidKnightMove(sx, sy, dx, dy);
//    case 'r': return isValidRookMove(sx, sy, dx, dy);
//    case 'b': return isValidBishopMove(sx, sy, dx, dy);
//    case 'q': return isValidQueenMove(sx, sy, dx, dy);
//    case 'k': return isValidKingMove(sx, sy, dx, dy);
//    }
//    return false;
//}
//
//void makeMove(int sx, int sy, int dx, int dy) {
//    board[dx][dy] = board[sx][sy];
//    board[sx][sy] = '.';
//}
//
//// ========== MAIN LOOP ==========
//
//int main() {
//    initializeBoard();
//
//    while (true) {
//        printBoard();
//        cout << (whiteTurn ? "White" : "Black") << "'s move: ";
//
//        string from, to;
//        cin >> from >> to;
//        if (from == "exit") break;
//
//        int fr, fc, tr, tc;
//        if (!algebraicToCoords(from, fr, fc) ||
//            !algebraicToCoords(to, tr, tc)) {
//            cout << "Invalid format. Use e.g. e2 e4\n";
//            continue;
//        }
//
//        if (isupper(board[fr][fc]) != whiteTurn) {
//            cout << "❌ Not your turn.\n";
//            continue;
//        }
//
//        if (isValidMove(fr, fc, tr, tc)) {
//            makeMove(fr, fc, tr, tc);
//            whiteTurn = !whiteTurn;
//        }
//        else {
//            cout << "❌ Invalid move! " << getHint(fr, fc, tr, tc) << "\n";
//        }
//    }
//
//    return 0;
//}
