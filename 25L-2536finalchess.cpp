//// chess_with_promotion_sidebar.cpp
// SFML chess with full movement rules, circular promotion menu (B3) and sidebar captured-tray (A2).
// Window size: 1000 x 800 (board 800x800 + sidebar 200x800).
// Build: link sfml-graphics, sfml-window, sfml-system. Use -std=c++17
// Note: This file uses windows.h for exe folder detection on Windows.

#include <SFML/Graphics.hpp>
#include <array>
#include <iostream>
#include <fstream>
#include <ctime>
#include <map>
#include <string>
#include <cctype>
#include <algorithm>
#include <vector>
#include <cmath>
#include <windows.h>

using Board = std::array<std::array<char, 8>, 8>;

// Window / board sizing
const int BOARD_PIX = 800;
const int SIDEBAR_PIX = 200;
const int WIN_W = BOARD_PIX + SIDEBAR_PIX;
const int WIN_H = 800;
const int BOARD_SIZE = 8;
const int SQUARE = BOARD_PIX / BOARD_SIZE;

// Colors
const sf::Color LIGHT_SQ(240, 217, 181);
const sf::Color DARK_SQ(181, 136, 99);
const sf::Color HIGHLIGHT_SELECT(100, 180, 255, 150);
const sf::Color HIGHLIGHT_VALID(100, 220, 100, 150);
const sf::Color HIGHLIGHT_INVALID(220, 80, 80, 160);
const sf::Color HOVER_OUTLINE(255, 255, 0, 200);
const sf::Color CHECK_OVERLAY(220, 50, 50, 170);
const sf::Color SIDEBAR_BG(28, 30, 33, 220);

// Player
enum class Player { White, Black };

// Helpers
std::string getExeFolder() {
    char buffer[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH) return {};
    std::string full(buffer, buffer + len);
    size_t pos = full.find_last_of("\\/");
    if (pos == std::string::npos) return {};
    return full.substr(0, pos);
}

bool insideBoard(int r, int c) { return r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE; }
bool isWhite(char p) { return std::isupper(static_cast<unsigned char>(p)); }
bool isBlack(char p) { return std::islower(static_cast<unsigned char>(p)); }

std::string pieceName(char p) {
    if (p == '.') return "Empty";
    char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(p)));
    std::string color = isWhite(p) ? "White " : "Black ";
    switch (lower) {
    case 'p': return color + "Pawn";
    case 'r': return color + "Rook";
    case 'n': return color + "Knight";
    case 'b': return color + "Bishop";
    case 'q': return color + "Queen";
    case 'k': return color + "King";
    default: return color + "Unknown";
    }
}

std::string algebraicFromRC(int r, int c) {
    if (!insideBoard(r, c)) return "??";
    char file = 'a' + c;
    char rank = '8' - r;
    return std::string() + file + rank;
}

Board makeStalemateTestBoard() {
    Board b{};
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            b[r][c] = '.';

    b[7][0] = 'K';  // White King at a1
    b[6][2] = 'k';  // Black King at c2
    b[5][1] = 'q';  // Black Queen at b3

    return b;
}

// Board init
Board makeStartingBoard() {
    Board b{};
    std::string backWhite = "RNBQKBNR";
    std::string backBlack = "rnbqkbnr";
    for (int i = 0; i < BOARD_SIZE; ++i) b[0][i] = backBlack[i];
    for (int i = 0; i < BOARD_SIZE; ++i) b[1][i] = 'p';
    for (int r = 2; r < 6; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c) b[r][c] = '.';
    for (int i = 0; i < BOARD_SIZE; ++i) b[6][i] = 'P';
    for (int i = 0; i < BOARD_SIZE; ++i) b[7][i] = backWhite[i];
    return b;
}

// piece filename mapping
std::string pieceToFilename(char piece) {
    bool white = isWhite(piece);
    char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(piece)));
    std::string prefix = white ? "w" : "b";
    char code = '?';
    switch (lower) {
    case 'p': code = 'p'; break;
    case 'r': code = 'r'; break;
    case 'n': code = 'n'; break;
    case 'b': code = 'b'; break;
    case 'q': code = 'q'; break;
    case 'k': code = 'k'; break;
    default: code = 'x'; break;
    }
    return prefix + std::string(1, code) + ".png";
}

bool tryLoadTexture(sf::Texture& tex, const std::string& relativePath, const std::string& exeFolder, std::string& outPathTried) {
    std::vector<std::string> candidates;
    // Simplified: just try direct paths
    candidates.push_back(relativePath);
    candidates.push_back("pieces/" + relativePath.substr(relativePath.find_last_of("/\\") + 1));
    candidates.push_back("../pieces/" + relativePath.substr(relativePath.find_last_of("/\\") + 1));
    for (auto& p : candidates) {
        if (tex.loadFromFile(p)) { outPathTried = p; return true; }
    }
    outPathTried = "";
    return false;
}

// ---------- Move generation (all pieces) ----------
void generateBasicMoves(const Board& board, int r0, int c0, Player turn, std::vector<sf::Vector2i>& out) {
    out.clear();
    if (!insideBoard(r0, c0)) return;
    char pc = board[r0][c0];
    if (pc == '.') return;
    bool pieceIsWhite = isWhite(pc);
    if ((turn == Player::White && !pieceIsWhite) || (turn == Player::Black && pieceIsWhite)) return;

    char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(pc)));
    auto pushIf = [&](int r, int c) {
        if (!insideBoard(r, c)) return;
        char dest = board[r][c];
        if (dest == '.') out.emplace_back(c, r);
        else {
            if (pieceIsWhite && isBlack(dest)) out.emplace_back(c, r);
            if (!pieceIsWhite && isWhite(dest)) out.emplace_back(c, r);
        }
        };

    if (lower == 'p') {
        int dir = pieceIsWhite ? -1 : 1;
        int r1 = r0 + dir;
        if (insideBoard(r1, c0) && board[r1][c0] == '.') {
            out.emplace_back(c0, r1);
            int startRow = pieceIsWhite ? 6 : 1;
            int r2 = r0 + 2 * dir;
            if (r0 == startRow && insideBoard(r2, c0) && board[r2][c0] == '.') out.emplace_back(c0, r2);
        }
        for (int dc : {-1, 1}) {
            int c1 = c0 + dc;
            if (insideBoard(r1, c1)) {
                char d = board[r1][c1];
                if (d != '.') {
                    if (pieceIsWhite && isBlack(d)) out.emplace_back(c1, r1);
                    if (!pieceIsWhite && isWhite(d)) out.emplace_back(c1, r1);
                }
            }
        }
    }
    else if (lower == 'n') {
        int drs[] = { -2,-2,-1,-1,1,1,2,2 };
        int dcs[] = { -1,1,-2,2,-2,2,-1,1 };
        for (int i = 0; i < 8; ++i) {
            int r = r0 + drs[i], c = c0 + dcs[i];
            if (!insideBoard(r, c)) continue;
            char d = board[r][c];
            if (d == '.') out.emplace_back(c, r);
            else if (pieceIsWhite && isBlack(d)) out.emplace_back(c, r);
            else if (!pieceIsWhite && isWhite(d)) out.emplace_back(c, r);
        }
    }
    else if (lower == 'k') {
        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                int r = r0 + dr, c = c0 + dc;
                pushIf(r, c);
            }
    }
    else if (lower == 'r' || lower == 'b' || lower == 'q') {
        std::vector<std::pair<int, int>> dirs;
        if (lower == 'r' || lower == 'q') {
            dirs.push_back({ 1,0 }); dirs.push_back({ -1,0 }); dirs.push_back({ 0,1 }); dirs.push_back({ 0,-1 });
        }
        if (lower == 'b' || lower == 'q') {
            dirs.push_back({ 1,1 }); dirs.push_back({ 1,-1 }); dirs.push_back({ -1,1 }); dirs.push_back({ -1,-1 });
        }
        for (auto d : dirs) {
            int r = r0 + d.first, c = c0 + d.second;
            while (insideBoard(r, c)) {
                char dest = board[r][c];
                if (dest == '.') {
                    out.emplace_back(c, r);
                }
                else {
                    if (pieceIsWhite && isBlack(dest)) out.emplace_back(c, r);
                    if (!pieceIsWhite && isWhite(dest)) out.emplace_back(c, r);
                    break;
                }
                r += d.first; c += d.second;
            }
        }
    }
}

// ---------- Check helpers ----------
bool findKing(const Board& board, Player kingColor, int& kr, int& kc) {
    char kch = (kingColor == Player::White) ? 'K' : 'k';
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            if (board[r][c] == kch) { kr = r; kc = c; return true; }
    return false;
}

bool squareAttacked(const Board& board, int tr, int tc, Player attacker) {
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            char pc = board[r][c];
            if (pc == '.') continue;
            bool isAttacker = (attacker == Player::White) ? isWhite(pc) : isBlack(pc);
            if (!isAttacker) continue;
            std::vector<sf::Vector2i> moves;
            generateBasicMoves(board, r, c, attacker, moves);
            for (auto& v : moves) {
                if (v.y == tr && v.x == tc) return true;
            }
        }
    }
    return false;
}

bool inCheck(const Board& board, Player who) {
    int kr, kc;
    if (!findKing(board, who, kr, kc)) return false;
    Player attacker = (who == Player::White) ? Player::Black : Player::White;
    return squareAttacked(board, kr, kc, attacker);
}

bool moveLeavesKingInCheck(const Board& board, int sr, int sc, int dr, int dc, Player mover) {
    Board copy = board;
    copy[dr][dc] = copy[sr][sc];
    copy[sr][sc] = '.';
    return inCheck(copy, mover);
}

bool hasAnyLegalMove(const Board& board, Player who) {
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            char pc = board[r][c];
            if (pc == '.') continue;
            if ((who == Player::White) != isWhite(pc)) continue;
            std::vector<sf::Vector2i> moves;
            generateBasicMoves(board, r, c, who, moves);
            for (auto& v : moves) {
                int dr = v.y, dc = v.x;
                if (!moveLeavesKingInCheck(board, r, c, dr, dc, who)) return true;
            }
        }
    }
    return false;
}

// ---------- Hint generator ----------
std::string getMoveHint(const Board& board, int sr, int sc, int dr, int dc, Player turn) {
    if (!insideBoard(sr, sc) || !insideBoard(dr, dc)) return "Coordinates out of range.";
    char src = board[sr][sc];
    if (src == '.') return "No piece on source square.";
    if ((turn == Player::White) != isWhite(src)) return "Not your piece.";
    std::vector<sf::Vector2i> candidates;
    generateBasicMoves(board, sr, sc, turn, candidates);
    bool geometryOk = false;
    for (auto& v : candidates) if (v.x == dc && v.y == dr) { geometryOk = true; break; }
    if (!geometryOk) {
        char p = std::tolower(static_cast<unsigned char>(src));
        switch (p) {
        case 'p':
            if (sc != dc) return "Pawn moves diagonally only when capturing.";
            return "Pawn cannot move that way (blocked or too far).";
        case 'n':
            return "Knight moves in an L-shape (2+1) and can jump over pieces.";
        case 'b':
            return "Bishop moves diagonally and cannot jump; path appears blocked or wrong direction.";
        case 'r':
            return "Rook moves in straight lines and cannot jump; path blocked or wrong direction.";
        case 'q':
            return "Queen moves like rook or bishop (straight or diagonal).";
        case 'k':
            return "King moves one square in any direction.";
        default:
            return "Invalid piece.";
        }
    }
    char dst = board[dr][dc];
    if (dst != '.' && (isWhite(dst) == isWhite(src))) return "You cannot capture your own piece.";
    if (moveLeavesKingInCheck(board, sr, sc, dr, dc, turn)) return "Move would leave your King in check.";
    return "Unknown reason (move invalid).";
}

// ---------- Promotion UI (circular) ----------
enum class PromoChoice { None, Queen, Rook, Bishop, Knight };

struct PromoMenu {
    bool active = false;
    int r = -1, c = -1;        // destination square (row,col) where pawn arrived
    Player color;              // color of pawn to promote
    // Button rectangles in window coordinates
    sf::FloatRect rectQ, rectR, rectB, rectN;
    // Draw helpers (computed each time)
    void open(int destR, int destC, Player who) {
        active = true; r = destR; c = destC; color = who;
    }
    void close() { active = false; r = c = -1; }
    // compute button rects given square center in pixel coordinates
    void computeRects(float cx, float cy, float s) {
        float b = s * 0.9f; // button size

        // Clamp center to keep buttons on screen
        float extent = s + b;
        if (cy - extent < 0) cy = extent + 10;
        if (cy + extent > BOARD_PIX) cy = BOARD_PIX - extent - 10;
        if (cx - extent < 0) cx = extent + 10;
        if (cx + extent > BOARD_PIX) cx = BOARD_PIX - extent - 10;

        rectQ = sf::FloatRect(cx - b / 2.f, cy - s - b * 0.9f, b, b); // top
        rectR = sf::FloatRect(cx - s - b * 0.9f, cy - b / 2.f, b, b); // left
        rectB = sf::FloatRect(cx + s * 0.0f + b * 0.0f, cy - b / 2.f, b, b); // right (slight offset)
        rectB.left = cx + s - b * 0.1f;
        rectN = sf::FloatRect(cx - b / 2.f, cy + s * 0.1f, b, b); // bottom
    }
    bool containsChoice(const sf::Vector2i& mp, PromoChoice& outChoice) {
        if (!active) { outChoice = PromoChoice::None; return false; }
        if (rectQ.contains((float)mp.x, (float)mp.y)) { outChoice = PromoChoice::Queen; return true; }
        if (rectR.contains((float)mp.x, (float)mp.y)) { outChoice = PromoChoice::Rook; return true; }
        if (rectB.contains((float)mp.x, (float)mp.y)) { outChoice = PromoChoice::Bishop; return true; }
        if (rectN.contains((float)mp.x, (float)mp.y)) { outChoice = PromoChoice::Knight; return true; }
        outChoice = PromoChoice::None; return false;
    }
} promoMenu;

// ---------- Main ----------
int main() {
    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Chess - Promotion Circle + Sidebar");
    window.setFramerateLimit(60);


   Board board = makeStartingBoard();
    Player turn = Player::White;

    //Board board = makeStalemateTestBoard();
    //Player turn = Player::Black;

    // textures
    std::map<char, sf::Texture> textures;
    std::map<char, bool> hasTexture;
    std::string exeFolder = getExeFolder();
    std::string piecesFolder = "pieces";
    std::string allPieces = "PRNBQKprnbqk";
    for (char pc : allPieces) {
        sf::Texture tex;
        std::string fname = piecesFolder + "/" + pieceToFilename(pc);
        std::string tried;
        if (tryLoadTexture(tex, fname, exeFolder, tried)) {
            textures[pc] = std::move(tex);
            hasTexture[pc] = true;
            std::cout << "Loaded " << pc << " from: " << tried << "\n";
        }
        else {
            hasTexture[pc] = false;
            std::cerr << "Failed to load " << pc << " (" << fname << ")\n";
        }
    }

    // promotion option textures (we can reuse Q,R,B,N of the player's color)
    // font
    sf::Font font;
    bool hasFont = font.loadFromFile("Roboto-Light.ttf");
    if (!hasFont) std::cerr << "Roboto-Light.ttf not found\n";

    // UI state
    bool selected = false;
    sf::Vector2i selectedRC(-1, -1);
    std::vector<sf::Vector2i> validTargets;
    bool dragging = false;
    sf::Sprite dragSprite;
    char dragPiece = '.';
    sf::Vector2i dragOrigin(-1, -1);
    sf::Vector2i hoverRC(-1, -1);

    float invalidFeedbackTimer = 0.f;
    sf::Vector2f invalidOffset(0.f, 0.f);

    // captured lists (store captured pieces as chars)
    std::vector<char> capturedByWhite; // white captured black pieces (show in sidebar)
    std::vector<char> capturedByBlack; // black captured white pieces

    // Scoring system (material count)
    auto calculateScore = [](const std::vector<char>& captured) {
        int score = 0;
        for (char p : captured) {
            char lower = std::tolower(p);
            if (lower == 'p') score += 1;
            else if (lower == 'n' || lower == 'b') score += 3;
            else if (lower == 'r') score += 5;
            else if (lower == 'q') score += 9;
        }
        return score;
        };

    // Function to save game score to file
    auto saveScoreToFile = [&](const std::string& result, int whiteScore, int blackScore) {
        std::ofstream outFile("game_scores.txt", std::ios::app); // Append mode
        if (outFile.is_open()) {
            // Get current time
            std::time_t now = std::time(nullptr);
            char timeStr[100];
            struct tm timeInfo;
            localtime_s(&timeInfo, &now); // Safe version for Windows
            std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeInfo);

            outFile << "===========================================\n";
            outFile << "Game Date: " << timeStr << "\n";
            outFile << "Result: " << result << "\n";
            outFile << "White Material Score: " << whiteScore << "\n";
            outFile << "Black Material Score: " << blackScore << "\n";
            outFile << "Material Difference: " << (whiteScore - blackScore) << "\n";
            outFile << "===========================================\n\n";
            outFile.close();
            std::cout << "Score saved to game_scores.txt\n";
        }
        else {
            std::cerr << "Unable to open file for saving score\n";
        }
        };

    bool gameOver = false;
    std::string gameStatus = "";

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) { window.close(); }

            // Mouse moved
            if (ev.type == sf::Event::MouseMoved) {
                sf::Vector2i mp = sf::Mouse::getPosition(window);
                int col = mp.x / SQUARE, row = mp.y / SQUARE;
                if (insideBoard(row, col)) hoverRC = { col, row }; else hoverRC = { -1, -1 };

                if (dragging) {
                    // center drag sprite on cursor
                    sf::Vector2u tsz = dragSprite.getTexture() ? dragSprite.getTexture()->getSize() : sf::Vector2u(SQUARE, SQUARE);
                    float texW = (float)tsz.x, texH = (float)tsz.y;
                    sf::Vector2f posf((float)mp.x - texW * dragSprite.getScale().x / 2.f,
                        (float)mp.y - texH * dragSprite.getScale().y / 2.f);
                    dragSprite.setPosition(posf);
                }
            }

            // Mouse button pressed
            if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mp(ev.mouseButton.x, ev.mouseButton.y);

                // If promotion menu active, let it handle clicks first
                if (promoMenu.active) {
                    PromoChoice choice;
                    if (promoMenu.containsChoice(mp, choice)) {
                        // perform promotion based on stored promoMenu.r/c
                        if (choice != PromoChoice::None) {
                            char promotedChar = 'Q';
                            if (choice == PromoChoice::Queen) promotedChar = (promoMenu.color == Player::White ? 'Q' : 'q');
                            if (choice == PromoChoice::Rook) promotedChar = (promoMenu.color == Player::White ? 'R' : 'r');
                            if (choice == PromoChoice::Bishop) promotedChar = (promoMenu.color == Player::White ? 'B' : 'b');
                            if (choice == PromoChoice::Knight) promotedChar = (promoMenu.color == Player::White ? 'N' : 'n');
                            // set square
                            board[promoMenu.r][promoMenu.c] = promotedChar;
                            std::cout << "Pawn promoted to " << pieceName(promotedChar) << " at " << algebraicFromRC(promoMenu.r, promoMenu.c) << "\n";
                            // close promo menu and switch turn
                            promoMenu.close();
                            // toggle turn unless game ended
                            turn = (turn == Player::White) ? Player::Black : Player::White;
                        }
                    }
                    else {
                        // clicks outside promotion menu ignored (could close menu if clicked outside)
                        // We'll ignore and keep waiting for promotion selection to be explicit.
                    }
                    continue;
                }

                // If click inside sidebar area -> could support clicking captured pieces later (ignore for now)
                if (mp.x >= BOARD_PIX) {
                    // clicking sidebar does nothing for now
                    continue;
                }

                // click on board
                int col = mp.x / SQUARE;
                int row = mp.y / SQUARE;
                if (!insideBoard(row, col)) continue;

                char pc = board[row][col];
                if (pc == '.') {
                    // clicking empty square clears selection
                    selected = false; validTargets.clear(); selectedRC = { -1, -1 };
                }
                else {
                    // only allow selecting piece of current player
                    if ((turn == Player::White && isWhite(pc)) || (turn == Player::Black && isBlack(pc))) {
                        selected = true; selectedRC = { col, row };
                        // compute targets & filter moves that leave king in check
                        generateBasicMoves(board, row, col, turn, validTargets);
                        std::vector<sf::Vector2i> filtered;
                        for (auto& v : validTargets) {
                            if (!moveLeavesKingInCheck(board, row, col, v.y, v.x, turn)) filtered.push_back(v);
                        }
                        validTargets = filtered;

                        // start drag
                        dragging = true;
                        dragOrigin = { col, row };
                        dragPiece = pc;
                        dragSprite = sf::Sprite();
                        if (hasTexture[pc]) {
                            dragSprite.setTexture(textures[pc]);
                            sf::Vector2u tsz = textures[pc].getSize();
                            float texW = (float)tsz.x, texH = (float)tsz.y;
                            float scale = std::min((float)SQUARE / texW, (float)SQUARE / texH) * 0.95f;
                            dragSprite.setScale(scale, scale);
                            dragSprite.setPosition((float)ev.mouseButton.x - texW * scale / 2.f,
                                (float)ev.mouseButton.y - texH * scale / 2.f);
                        }
                        else {
                            // fallback circle rendered to texture
                            sf::RenderTexture rt; rt.create(SQUARE, SQUARE); rt.clear(sf::Color::Transparent);
                            sf::CircleShape circle((float)SQUARE * 0.36f);
                            circle.setOrigin(circle.getRadius(), circle.getRadius());
                            circle.setPosition(SQUARE / 2.f, SQUARE / 2.f);
                            circle.setFillColor(isWhite(pc) ? sf::Color(230, 230, 230) : sf::Color(50, 50, 50));
                            circle.setOutlineColor(sf::Color::Black); circle.setOutlineThickness(2.f); rt.draw(circle);
                            if (hasFont) {
                                sf::Text t; t.setFont(font); t.setString(std::string(1, (char)std::toupper(pc)));
                                t.setCharacterSize(SQUARE / 3); t.setFillColor(isWhite(pc) ? sf::Color::Black : sf::Color::White);
                                sf::FloatRect tb = t.getLocalBounds(); t.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
                                t.setPosition(SQUARE / 2.f, SQUARE / 2.f + 6); rt.draw(t);
                            }
                            rt.display();
                            dragSprite.setTexture(rt.getTexture());
                            dragSprite.setScale(1.f, 1.f);
                            dragSprite.setPosition((float)ev.mouseButton.x - SQUARE / 2.f, (float)ev.mouseButton.y - SQUARE / 2.f);
                        }
                    }
                    else {
                        // clicking opponent piece - optional message
                        // std::cout << "Not your turn for that piece.\n";
                    }
                }
            } // mouse pressed

            // Mouse button released
            if (ev.type == sf::Event::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left && dragging) {
                sf::Vector2i mp(ev.mouseButton.x, ev.mouseButton.y);
                // if drop in sidebar - treat as cancel
                if (mp.x >= BOARD_PIX) {
                    std::cout << "Dropped on sidebar -> cancelled\n";
                    invalidFeedbackTimer = 0.35f;
                    dragging = false; dragPiece = '.'; dragOrigin = { -1,-1 };
                    continue;
                }
                int col = mp.x / SQUARE, row = mp.y / SQUARE;
                bool placed = false;
                if (insideBoard(row, col)) {
                    bool ok = false;
                    for (auto& v : validTargets) if (v.x == col && v.y == row) { ok = true; break; }
                    if (ok) {
                        // perform move, capture if any
                        char dest = board[row][col];
                        if (dest != '.') {
                            if (isWhite(dest)) capturedByBlack.push_back(dest); else capturedByWhite.push_back(dest);
                        }
                        board[row][col] = dragPiece;
                        board[dragOrigin.y][dragOrigin.x] = '.';
                        std::cout << "Moved " << pieceName(dragPiece) << " from " << algebraicFromRC(dragOrigin.y, dragOrigin.x)
                            << " to " << algebraicFromRC(row, col) << "\n";

                        // If pawn reached final rank -> open promo menu (do not switch turn yet)
                        bool promoting = false;
                        if ((dragPiece == 'P' && row == 0) || (dragPiece == 'p' && row == 7)) promoting = true;

                        if (promoting) {
                            // Open circular promotion menu around (row,col)
                            promoMenu.open(row, col, turn);
                            // compute pixel center of that square and rects for selection
                            float cx = (col + 0.5f) * (float)SQUARE;
                            float cy = (row + 0.5f) * (float)SQUARE;
                            float gap = (float)SQUARE * 0.9f;
                            promoMenu.computeRects(cx, cy, (float)SQUARE * 0.9f);
                            // keep the pawn there until player picks promotion; do NOT toggle turn
                            std::cout << "Promotion menu opened at " << algebraicFromRC(row, col) << "\n";
                        }
                        else {
                            // normal move -> toggle turn
                            turn = (turn == Player::White) ? Player::Black : Player::White;
                            // after toggling, check for check/checkmate/stalemate
                            // 'turn' is now the player who must respond
                            Player currentPlayer = turn;
                            bool inCheckNow = inCheck(board, currentPlayer);
                            bool hasMove = hasAnyLegalMove(board, currentPlayer);
                            if (inCheckNow) {
                                if (!hasMove) {
                                    // Current player is in checkmate, so the OTHER player wins
                                    gameStatus = (currentPlayer == Player::White ? "CHECKMATE - Black wins" : "CHECKMATE - White wins");
                                    gameOver = true;
                                    // Save score to file
                                    int whiteScore = calculateScore(capturedByWhite);
                                    int blackScore = calculateScore(capturedByBlack);
                                    saveScoreToFile(gameStatus, whiteScore, blackScore);
                                }
                                else { gameStatus = "CHECK"; std::cout << "CHECK\n"; }
                            }
                            else {
                                if (!hasMove) {
                                    gameStatus = "STALEMATE - Draw";
                                    gameOver = true;
                                    // Save score to file
                                    int whiteScore = calculateScore(capturedByWhite);
                                    int blackScore = calculateScore(capturedByBlack);
                                    saveScoreToFile(gameStatus, whiteScore, blackScore);
                                }
                                else gameStatus = "";
                            }
                        }

                        placed = true;
                    }
                    else {
                        std::string hint = getMoveHint(board, dragOrigin.y, dragOrigin.x, row, col, turn);
                        std::cout << "Invalid drop to " << algebraicFromRC(row, col) << " -> " << hint << "\n";
                        invalidFeedbackTimer = 0.35f;
                    }
                }
                else {
                    std::cout << "Dropped off-board -> cancel\n";
                    invalidFeedbackTimer = 0.35f;
                }

                dragging = false;
                dragPiece = '.'; dragOrigin = { -1, -1 };
                // Always clear selection state when drag ends, not just on successful placement
                selected = false; validTargets.clear(); selectedRC = { -1, -1 };
            } // mouse released

            // If promo menu active and window receives events, we handled clicks above already

            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Escape) {
                    if (dragging) { dragging = false; dragPiece = '.'; dragOrigin = { -1,-1 }; }
                }
            }

            if (ev.type == sf::Event::LostFocus) {
                if (dragging) { dragging = false; dragPiece = '.'; dragOrigin = { -1,-1 }; }
            }
        } // event processing end

        // invalid shake update
        if (invalidFeedbackTimer > 0.f) {
            invalidFeedbackTimer -= 1.f / 60.f;
            float t = invalidFeedbackTimer;
            invalidOffset.x = std::sin(t * 50.f) * 6.f;
            if (invalidFeedbackTimer <= 0.f) { invalidOffset = { 0.f,0.f }; invalidFeedbackTimer = 0.f; }
        }

        // draw
        window.clear();

        // apply shake by moving view
        sf::View originalView = window.getView();
        sf::View view = originalView; view.move(invalidOffset); window.setView(view);

        // board squares
        for (int r = 0; r < BOARD_SIZE; ++r) {
            for (int c = 0; c < BOARD_SIZE; ++c) {
                bool light = ((r + c) % 2 == 0);
                sf::RectangleShape rect(sf::Vector2f((float)SQUARE, (float)SQUARE));
                rect.setPosition((float)(c * SQUARE), (float)(r * SQUARE));
                rect.setFillColor(light ? LIGHT_SQ : DARK_SQ);
                window.draw(rect);
            }
        }

        // Draw Rank and File labels
        if (hasFont) {
            sf::Text lbl; lbl.setFont(font); lbl.setCharacterSize(14);
            // Files a-h (bottom)
            for (int c = 0; c < 8; ++c) {
                lbl.setString(std::string(1, 'a' + c));
                bool light = ((7 + c) % 2 == 0); // Bottom row is rank 1 (index 7)
                lbl.setFillColor(light ? DARK_SQ : LIGHT_SQ); // Contrast
                lbl.setPosition(c * SQUARE + SQUARE - 15, 7 * SQUARE + SQUARE - 20);
                window.draw(lbl);
            }
            // Ranks 1-8 (left)
            for (int r = 0; r < 8; ++r) {
                lbl.setString(std::to_string(8 - r));
                bool light = ((r + 0) % 2 == 0); // Left col is file a (index 0)
                lbl.setFillColor(light ? DARK_SQ : LIGHT_SQ); // Contrast
                lbl.setPosition(2, r * SQUARE + 2);
                window.draw(lbl);
            }
        }

        // draw valid targets
        for (auto& v : validTargets) {
            sf::RectangleShape r(sf::Vector2f((float)SQUARE, (float)SQUARE));
            r.setPosition((float)(v.x * SQUARE), (float)(v.y * SQUARE));
            r.setFillColor(HIGHLIGHT_VALID);
            window.draw(r);
        }

        // selection highlight
        if (selected && insideBoard(selectedRC.y, selectedRC.x)) {
            sf::RectangleShape sel(sf::Vector2f((float)SQUARE, (float)SQUARE));
            sel.setPosition((float)(selectedRC.x * SQUARE), (float)(selectedRC.y * SQUARE));
            sel.setFillColor(HIGHLIGHT_SELECT);
            window.draw(sel);
        }

        // highlight king if in check (use gameStatus variable)
        if (gameStatus == "CHECK" || gameStatus.rfind("CHECKMATE", 0) == 0) {
            Player inCheckPlayer = (turn == Player::White) ? Player::Black : Player::White;
            int kr, kc;
            if (findKing(board, inCheckPlayer, kr, kc)) {
                sf::RectangleShape ko(sf::Vector2f((float)SQUARE, (float)SQUARE));
                ko.setPosition((float)(kc * SQUARE), (float)(kr * SQUARE));
                ko.setFillColor(CHECK_OVERLAY);
                window.draw(ko);
            }
        }

        // hover outline
        if (hoverRC.x != -1 && hoverRC.y != -1) {
            sf::RectangleShape ho(sf::Vector2f((float)SQUARE, (float)SQUARE));
            ho.setPosition((float)(hoverRC.x * SQUARE), (float)(hoverRC.y * SQUARE));
            ho.setFillColor(sf::Color::Transparent);
            ho.setOutlineThickness(2.f);
            ho.setOutlineColor(HOVER_OUTLINE);
            window.draw(ho);
        }

        // draw pieces (skip dragging origin)
        for (int r = 0; r < BOARD_SIZE; ++r) {
            for (int c = 0; c < BOARD_SIZE; ++c) {
                char pc = board[r][c];
                if (pc == '.') continue;
                if (dragging && dragOrigin.x == c && dragOrigin.y == r) continue;
                if (hasTexture[pc]) {
                    sf::Sprite spr; spr.setTexture(textures[pc]);
                    sf::Vector2u tsz = textures[pc].getSize();
                    float texW = (float)tsz.x, texH = (float)tsz.y;
                    float scale = std::min((float)SQUARE / texW, (float)SQUARE / texH) * 0.95f;
                    spr.setScale(scale, scale);
                    spr.setPosition(c * SQUARE + (SQUARE - texW * scale) / 2.f, r * SQUARE + (SQUARE - texH * scale) / 2.f);
                    window.draw(spr);
                }
                else {
                    sf::CircleShape circle((float)SQUARE * 0.36f);
                    circle.setOrigin(circle.getRadius(), circle.getRadius());
                    circle.setPosition(c * SQUARE + SQUARE / 2.f, r * SQUARE + SQUARE / 2.f);
                    circle.setFillColor(isWhite(pc) ? sf::Color(230, 230, 230) : sf::Color(50, 50, 50));
                    circle.setOutlineColor(sf::Color::Black); circle.setOutlineThickness(2.f);
                    window.draw(circle);
                    if (hasFont) {
                        sf::Text t; t.setFont(font); t.setString(std::string(1, (char)std::toupper(pc)));
                        t.setCharacterSize(SQUARE / 3); t.setFillColor(isWhite(pc) ? sf::Color::Black : sf::Color::White);
                        sf::FloatRect tb = t.getLocalBounds(); t.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
                        t.setPosition(c * SQUARE + SQUARE / 2.f, r * SQUARE + SQUARE / 2.f + 6);
                        window.draw(t);
                    }
                }
            }
        }

        // draw dragging sprite on top (still in shaky view)
        if (dragging) {
            window.draw(dragSprite);
        }

        /*
         FIX: Sidebar MUST be drawn in normal view.
         Otherwise it is shifted and appears blank.
        */
        window.setView(window.getDefaultView());

        // draw sidebar background (NOW correct)
        sf::RectangleShape sidebar(sf::Vector2f((float)SIDEBAR_PIX, (float)WIN_H));
        sidebar.setPosition((float)BOARD_PIX, 0.f);
        sidebar.setFillColor(SIDEBAR_BG);
        window.draw(sidebar);

        // draw sidebar content: headings and captured sprites
        if (hasFont) {
            // White's captures (black pieces)
            sf::Text t; t.setFont(font); t.setCharacterSize(16); t.setFillColor(sf::Color::White);
            t.setString("White Captured"); t.setPosition(BOARD_PIX + 12.f, 12.f); window.draw(t);

            int whiteScore = calculateScore(capturedByWhite);
            sf::Text scoreW; scoreW.setFont(font); scoreW.setCharacterSize(14); scoreW.setFillColor(sf::Color(100, 255, 100));
            scoreW.setString("+" + std::to_string(whiteScore)); scoreW.setPosition(BOARD_PIX + 12.f, 32.f); window.draw(scoreW);

            // Display captured pieces in a grid to prevent overlap
            float y = 55.f;
            float x = BOARD_PIX + 12.f;
            float iconSize = 28.f;
            int col = 0;
            int maxCols = 5;

            for (char pc : capturedByWhite) {
                if (hasTexture[pc]) {
                    sf::Sprite s; s.setTexture(textures[pc]);
                    sf::Vector2u tsz = textures[pc].getSize();
                    float scale = std::min(iconSize / tsz.x, iconSize / tsz.y);
                    s.setScale(scale, scale);
                    s.setPosition(x + col * (iconSize + 4), y);
                    window.draw(s);
                }
                else {
                    sf::RectangleShape r(sf::Vector2f(iconSize, iconSize));
                    r.setPosition(x + col * (iconSize + 4), y);
                    r.setFillColor(sf::Color::White); window.draw(r);
                }
                col++;
                if (col >= maxCols) { col = 0; y += iconSize + 4; }
            }

            // Black's captures (white pieces)
            y = std::max(y + iconSize + 20.f, 250.f);
            sf::Text t2; t2.setFont(font); t2.setCharacterSize(16); t2.setFillColor(sf::Color::White);
            t2.setString("Black Captured"); t2.setPosition(BOARD_PIX + 12.f, y); window.draw(t2);

            int blackScore = calculateScore(capturedByBlack);
            sf::Text scoreB; scoreB.setFont(font); scoreB.setCharacterSize(14); scoreB.setFillColor(sf::Color(100, 255, 100));
            scoreB.setString("+" + std::to_string(blackScore)); scoreB.setPosition(BOARD_PIX + 12.f, y + 20.f); window.draw(scoreB);

            y += 45.f;
            col = 0;
            for (char pc : capturedByBlack) {
                if (hasTexture[pc]) {
                    sf::Sprite s; s.setTexture(textures[pc]);
                    sf::Vector2u tsz = textures[pc].getSize();
                    float scale = std::min(iconSize / tsz.x, iconSize / tsz.y);
                    s.setScale(scale, scale);
                    s.setPosition(x + col * (iconSize + 4), y);
                    window.draw(s);
                }
                else {
                    sf::RectangleShape r(sf::Vector2f(iconSize, iconSize));
                    r.setPosition(x + col * (iconSize + 4), y);
                    r.setFillColor(sf::Color::White); window.draw(r);
                }
                col++;
                if (col >= maxCols) { col = 0; y += iconSize + 4; }
            }


            // turn & status
            sf::Text t3; t3.setFont(font); t3.setCharacterSize(16); t3.setFillColor(sf::Color::White);
            t3.setString("Turn: " + std::string((turn == Player::White) ? "White" : "Black"));
            t3.setPosition(BOARD_PIX + 12.f, 640.f); window.draw(t3);

            // Status text with adaptive sizing to prevent overflow
            sf::Text t4; t4.setFont(font); t4.setFillColor(sf::Color::White);
            std::string statusText = gameStatus.empty() ? "OK" : gameStatus;
            // Use smaller font for long messages
            if (statusText.length() > 15) {
                t4.setCharacterSize(11);
            }
            else {
                t4.setCharacterSize(14);
            }
            t4.setString(statusText);
            t4.setPosition(BOARD_PIX + 12.f, 668.f);
            window.draw(t4);

            sf::Text t5; t5.setFont(font); t5.setCharacterSize(10); t5.setFillColor(sf::Color(180, 180, 180));
            t5.setString("Drag pieces to move"); t5.setPosition(BOARD_PIX + 12.f, 700.f); window.draw(t5);
        }

        // draw promotion menu if active
        if (promoMenu.active) {
            // dim board slightly
            sf::RectangleShape overlay(sf::Vector2f((float)BOARD_PIX, (float)BOARD_PIX));
            overlay.setPosition(0.f, 0.f);
            overlay.setFillColor(sf::Color(0, 0, 0, 100));
            window.draw(overlay);

            // compute pixel center for square
            float cx = (promoMenu.c + 0.5f) * (float)SQUARE;
            float cy = (promoMenu.r + 0.5f) * (float)SQUARE;
            promoMenu.computeRects(cx, cy, (float)SQUARE * 0.9f);

            // draw circular arrangement - draw each button as a rounded rectangle or circle with piece sprite
            // top: Queen
            sf::CircleShape btnQ(promoMenu.rectQ.width / 2.f); btnQ.setOrigin(btnQ.getRadius(), btnQ.getRadius());
            btnQ.setPosition(promoMenu.rectQ.left + promoMenu.rectQ.width / 2.f, promoMenu.rectQ.top + promoMenu.rectQ.height / 2.f);
            btnQ.setFillColor(sf::Color(220, 220, 220)); btnQ.setOutlineColor(sf::Color::Black); btnQ.setOutlineThickness(2.f);
            window.draw(btnQ);
            // left: Rook
            sf::CircleShape btnR(promoMenu.rectR.width / 2.f); btnR.setOrigin(btnR.getRadius(), btnR.getRadius());
            btnR.setPosition(promoMenu.rectR.left + promoMenu.rectR.width / 2.f, promoMenu.rectR.top + promoMenu.rectR.height / 2.f);
            btnR.setFillColor(sf::Color(220, 220, 220)); btnR.setOutlineColor(sf::Color::Black); btnR.setOutlineThickness(2.f);
            window.draw(btnR);
            // right: Bishop
            sf::CircleShape btnB(promoMenu.rectB.width / 2.f); btnB.setOrigin(btnB.getRadius(), btnB.getRadius());
            btnB.setPosition(promoMenu.rectB.left + promoMenu.rectB.width / 2.f, promoMenu.rectB.top + promoMenu.rectB.height / 2.f);
            btnB.setFillColor(sf::Color(220, 220, 220)); btnB.setOutlineColor(sf::Color::Black); btnB.setOutlineThickness(2.f);
            window.draw(btnB);
            // bottom: Knight
            sf::CircleShape btnN(promoMenu.rectN.width / 2.f); btnN.setOrigin(btnN.getRadius(), btnN.getRadius());
            btnN.setPosition(promoMenu.rectN.left + promoMenu.rectN.width / 2.f, promoMenu.rectN.top + promoMenu.rectN.height / 2.f);
            btnN.setFillColor(sf::Color(220, 220, 220)); btnN.setOutlineColor(sf::Color::Black); btnN.setOutlineThickness(2.f);
            window.draw(btnN);

            // draw labels/sprites inside buttons
            // choose color prefix
            char qch = (promoMenu.color == Player::White) ? 'Q' : 'q';
            char rch = (promoMenu.color == Player::White) ? 'R' : 'r';
            char bch = (promoMenu.color == Player::White) ? 'B' : 'b';
            char nch = (promoMenu.color == Player::White) ? 'N' : 'n';

            auto drawSmallSprite = [&](char piece, const sf::FloatRect& rect) {
                if (hasTexture[piece]) {
                    sf::Sprite s; s.setTexture(textures[piece]);
                    sf::Vector2u tsz = textures[piece].getSize();
                    float maxs = rect.width * 0.85f;
                    float scale = std::min(maxs / tsz.x, maxs / tsz.y);
                    s.setScale(scale, scale);
                    s.setPosition(rect.left + (rect.width - tsz.x * scale) / 2.f, rect.top + (rect.height - tsz.y * scale) / 2.f);
                    window.draw(s);
                }
                else if (hasFont) {
                    sf::Text t; t.setFont(font); t.setCharacterSize((unsigned int)(rect.width * 0.5f));
                    t.setString(std::string(1, std::toupper(piece)));
                    t.setFillColor(sf::Color::Black);
                    sf::FloatRect tb = t.getLocalBounds();
                    t.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
                    t.setPosition(rect.left + rect.width / 2.f, rect.top + rect.height / 2.f - 4.f);
                    window.draw(t);
                }
                };

            drawSmallSprite(qch, promoMenu.rectQ);
            drawSmallSprite(rch, promoMenu.rectR);
            drawSmallSprite(bch, promoMenu.rectB);
            drawSmallSprite(nch, promoMenu.rectN);
        }

        window.display();
    }

    return 0;
}
