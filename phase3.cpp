//
//#include <iostream>
//#include <string>
//#include <cctype>
//using namespace std;
//
// board size
//const int SIZE = 8;
//
// the chessboard
//char board[SIZE][SIZE];
//
// true → white's turn, false → black's turn
//bool whiteTurn = true;
//
//
// ------------------------------
// Helper: Convert "e2" to (row,col)
// ------------------------------
//bool convertToCoords(string move, int& row, int& col) {
//    if (move.length() != 2) return false;
//
//    char file = tolower(move[0]); // a-h
//    char rank = move[1];          // 1-8
//
//    if (file < 'a' || file > 'h') return false;
//    if (rank < '1' || rank > '8') return false;
//
//    col = file - 'a';           // 'a' → 0
//    row = 8 - (rank - '0');     // '8' → 0, '1' → 7
//
//    return true;
//}
//
//
// ------------------------------
// Initialize board
// ------------------------------
//void initializeBoard() {
//    string whiteBack = "RNBQKBNR";
//    string blackBack = "rnbqkbnr";
//
//     Black pieces
//    for (int c = 0; c < SIZE; c++) board[0][c] = blackBack[c];
//    for (int c = 0; c < SIZE; c++) board[1][c] = 'p';
//
//     Empty squares
//    for (int r = 2; r < 6; r++)
//        for (int c = 0; c < SIZE; c++)
//            board[r][c] = '.';
//
//     White pieces
//    for (int c = 0; c < SIZE; c++) board[6][c] = 'P';
//    for (int c = 0; c < SIZE; c++) board[7][c] = whiteBack[c];
//}
//
//
// ------------------------------
// Display the board
// ------------------------------
//void printBoard() {
//    cout << "   a b c d e f g h\n";
//
//    for (int r = 0; r < SIZE; r++) {
//        cout << 8 - r << "  ";
//
//        for (int c = 0; c < SIZE; c++) {
//            cout << board[r][c] << " ";
//        }
//
//        cout << " " << 8 - r << "\n";
//    }
//
//    cout << "   a b c d e f g h\n\n";
//}
//
//
// ------------------------------
// Pawn movement rules
// ------------------------------
//bool validPawnMove(int sr, int sc, int dr, int dc) {
//    char piece = board[sr][sc];
//    bool isWhite = isupper(piece);
//
//    int direction = isWhite ? -1 : 1;
//
//    ;
//     Move 1 square forward
//    if (dr == sr + direction && dc == sc && board[dr][dc] == '.') {
//        return true;
//    }
//
//     Move 2 squares forward from start
//    if (isWhite && sr == 6 && dr == 4 && dc == sc && board[5][sc] == '.' && board[4][sc] == '.')
//        return true;
//
//    if (!isWhite && sr == 1 && dr == 3 && dc == sc && board[2][sc] == '.' && board[3][sc] == '.')
//        return true;
//
//     Diagonal capture
//    if (dr == sr + direction && (dc == sc + 1 || dc == sc - 1)) 
//    {
//        if (board[dr][dc] != '.' && isupper(piece) != isupper(board[dr][dc]))
//            return true;
//    }
//
//    return false;
//}
//
//
// ------------------------------
// Bishop movement rules
// ------------------------------
//bool validBishopMove(int sr, int sc, int dr, int dc) {
//    if (abs(dr - sr) != abs(dc - sc))
//        return false; // must be diagonal
//
//    int rStep = (dr > sr) ? 1 : -1;
//    int cStep = (dc > sc) ? 1 : -1;
//
//    int r = sr + rStep;
//    int c = sc + cStep;
//
//    while (r != dr && c != dc) {
//        if (board[r][c] != '.') return false;
//        r += rStep;
//        c += cStep;
//    }
//
//     Final square
//    char src = board[sr][sc];
//    char dst = board[dr][dc];
//
//     empty or enemy
//    return dst == '.' || isupper(src) != isupper(dst);
//}
//
//
// ------------------------------
// Check if move is valid
// ------------------------------
//bool isValidMove(int sr, int sc, int dr, int dc) {
//    char piece = board[sr][sc];
//
//    if (piece == '.') return false;
//    if (isupper(piece) != whiteTurn) return false; // wrong turn
//
//    char p = tolower(piece);
//
//    if (p == 'p') return validPawnMove(sr, sc, dr, dc);
//    if (p == 'b') return validBishopMove(sr, sc, dr, dc);
//
//    return false; // other pieces not implemented
//}
//
//
// ------------------------------
// Make move
// ------------------------------
//void makeMove(int sr, int sc, int dr, int dc) {
//    board[dr][dc] = board[sr][sc];
//    board[sr][sc] = '.';
//}
//
//
// ------------------------------
// Main game loop
// ------------------------------
//int main() {
//    initializeBoard();
//
//    while (true) {
//        printBoard();
//        cout << (whiteTurn ? "White" : "Black") << "'s move: ";
//
//        string from, to;
//        cin >> from >> to;
//
//        if (from == "exit") break;
//
//        int sr, sc, dr, dc;
//        if (!convertToCoords(from, sr, sc) ||
//            !convertToCoords(to, dr, dc)) {
//            cout << "Invalid input format!\n";
//            continue;
//        }
//
//        if (isValidMove(sr, sc, dr, dc)) {
//            makeMove(sr, sc, dr, dc);
//            whiteTurn = !whiteTurn;
//        }
//        else {
//            cout << "Invalid move!\n";
//        }
//    }
//
//    return 0;
//}