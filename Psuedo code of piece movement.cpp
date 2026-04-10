// -------------------------------------
// Task 4 – Pseudocode for Knight / Rook / Queen / King
// -------------------------------------
/*
bool isValidKnightMove(...) {
    PSEUDOCODE:
    - Knight moves in L-shape:
        if ( (abs(dx-sx)==2 && abs(dy-sy)==1) OR (abs(dx-sx)==1 && abs(dy-sy)==2) )
            if destination empty OR enemy piece → valid
        else invalid
}
bool isValidRookMove(...) {
    PSEUDOCODE:
    - If same row OR same column:
         step through squares between source and destination
         if any square blocked → return false
    - If destination empty or enemy → true
}
bool isValidQueenMove(...) {
    PSEUDOCODE:
    - Queen = rook + bishop logic combined
    - If diagonal -> use bishop's check
    - If straight -> use rook's check
}
bool isValidKingMove(...) {
    PSEUDOCODE:
    - King can move 1 square in any direction
    - If abs(dx-sx)<=1 and abs(dy-sy)<=1:
         ensure destination empty/enemy
         return true
    - else false
}
*/

// -------------------------------------
// Task 5 – Check / Checkmate / Stalemate (Pseudocode)
// -------------------------------------
/*
isCheck(color):
    - Locate king of given color
    - For every opponent piece:
         generate its legal moves
         if any move attacks king → return true
    return false

isCheckmate(color):
    - If king is in check AND no legal move exists:
         return true

isStalemate(color):
    - If king NOT in check BUT no legal moves:
         return true
*/