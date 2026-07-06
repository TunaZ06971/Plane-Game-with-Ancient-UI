/*
 * File: Display.cpp
 * Purpose: Implements ncurses-based layout computation and rendering for every game state.
 */
#include "Display.h"
#include <cstring>
#include <algorithm>

RenderOffsets gRenderOffsets{1, 1};

/*
 * Purpose: Construct a Display helper; heavy work occurs during init().
 * Inputs: None.
 * Returns: Display instance.
 */
Display::Display() {}

/*
 * Purpose: Default destructor; cleanup occurs via explicit cleanup().
 * Inputs: None.
 * Returns: Nothing.
 */
Display::~Display() {
    // cleanup is called explicitly in Game loop or at exit, but safe to ensure here
}

/*
 * Purpose: Initialize ncurses state and color pairs for all rendering.
 * Inputs: None.
 * Returns: Nothing.
 */
void Display::init() {
    // Initialize ncurses, use cbreak instead of raw to maintain normal terminal behavior
    initscr();
    cbreak();           // Use cbreak instead of raw, won't completely take over terminal
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);
    timeout(0);
    nodelay(stdscr, TRUE);  // Non-blocking input
    
    // Set colors (if supported)
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN,  COLOR_BLACK);  // Player
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Player bullet (yellow)
        init_pair(3, COLOR_RED,    COLOR_BLACK);  // Solid obstacle / Boss HP bar
        init_pair(4, COLOR_WHITE,  COLOR_BLACK);  // Text
        init_pair(5, COLOR_YELLOW, COLOR_BLACK);  // Breakable obstacle
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK); // Explode obstacle
        init_pair(7, COLOR_BLUE,   COLOR_BLACK);  // Damage buff (base)
        // Approximate orange for boss/enemy bullets
        init_pair(8, COLOR_YELLOW, COLOR_RED);
    }
}

/*
 * Purpose: Restore the terminal when the game exits.
 * Inputs: None.
 * Returns: Nothing.
 */
void Display::cleanup() {
    endwin();
}

/*
 * Purpose: Compute offsets that center the logical window inside the terminal.
 * Inputs: None.
 * Returns: Nothing; updates gRenderOffsets and cached border coordinates.
 */
void Display::updateLayout() {
    int termHeight = 0;
    int termWidth = 0;
    getmaxyx(stdscr, termHeight, termWidth);

    int fullHeight = WINDOW_HEIGHT + 2;
    int fullWidth = WINDOW_WIDTH + 2;

    int verticalSpace = termHeight - fullHeight;
    int horizontalSpace = termWidth - fullWidth;

    borderTop = verticalSpace > 0 ? verticalSpace / 2 : 0;
    borderLeft = horizontalSpace > 0 ? horizontalSpace / 2 : 0;

    if (borderTop == 0 && termHeight > fullHeight) {
        borderTop = 1;
    }

    if (borderTop + fullHeight > termHeight) {
        borderTop = std::max(0, termHeight - fullHeight);
    }

    borderTop = std::max(0, borderTop);

    if (borderLeft + fullWidth > termWidth) {
        borderLeft = std::max(0, termWidth - fullWidth);
    }

    borderLeft = std::max(0, borderLeft);

    borderBottom = std::min(termHeight - 1, borderTop + fullHeight - 1);
    borderRight = std::min(termWidth - 1, borderLeft + fullWidth - 1);

    gRenderOffsets.x = borderLeft + 1;
    gRenderOffsets.y = borderTop + 1;

    if (gRenderOffsets.y <= 0) {
        gRenderOffsets.y = 1;
    }
}

/*
 * Purpose: Draw an outer border representing the playable window.
 * Inputs: None.
 * Returns: Nothing.
 */
void Display::drawGameBorder() const {
    if (borderBottom <= borderTop || borderRight <= borderLeft) {
        return;
    }

    mvaddch(borderTop, borderLeft, '+');
    mvaddch(borderTop, borderRight, '+');
    mvaddch(borderBottom, borderLeft, '+');
    mvaddch(borderBottom, borderRight, '+');

    if (borderRight - borderLeft > 1) {
        mvhline(borderTop, borderLeft + 1, '-', borderRight - borderLeft - 1);
        mvhline(borderBottom, borderLeft + 1, '-', borderRight - borderLeft - 1);
    }

    if (borderBottom - borderTop > 1) {
        mvvline(borderTop + 1, borderLeft, '|', borderBottom - borderTop - 1);
        mvvline(borderTop + 1, borderRight, '|', borderBottom - borderTop - 1);
    }
}

/*
 * Purpose: Render a frame according to the current game state.
 * Inputs: Game state, entity references, stats, and menu selections.
 * Returns: Nothing.
 */
void Display::draw(GameState currentState, 
          const Player& player, 
          const std::vector<std::unique_ptr<Bullet>>& bullets,
          const std::vector<std::unique_ptr<Enemy>>& enemies,
          const Boss* boss,
          int score, 
          int enemiesDefeated, 
          int playerLives, 
          int bulletDamage, 
          Difficulty currentDifficulty, 
          bool autoShoot,
          int selectedButton,
          const std::vector<int>& leaderboard) {
              
    clear();
    updateLayout();
    drawGameBorder();

    switch (currentState) {
        case GameState::START_MENU:
            drawMenu();
            break;
        case GameState::DIFFICULTY_SELECT:
            drawDifficultySelect(selectedButton);
            break;
        case GameState::PLAYING:
            drawPlaying(player, bullets, enemies, boss, score, enemiesDefeated, playerLives, bulletDamage, currentDifficulty, autoShoot);
            break;
        case GameState::WIN:
            drawWin();
            break;
        case GameState::GAME_OVER:
            drawGameOver();
            break;
        case GameState::STATS_VIEW:
            drawStats(currentDifficulty, leaderboard, score, selectedButton);
            break;
    }
    
    refresh();
}

/*
 * Purpose: Render the start menu with the play prompt.
 * Inputs: None.
 * Returns: Nothing.
 */
void Display::drawMenu() {
    mvprintw(translateY(WINDOW_HEIGHT / 3), translateX(WINDOW_WIDTH / 2 - 5), "PLANE FIGHT");
    
    // Draw Play button (simulated)
    mvprintw(translateY(WINDOW_HEIGHT / 2), translateX(WINDOW_WIDTH / 2 - 4), "[ PLAY ]");
    mvprintw(translateY(WINDOW_HEIGHT / 2 + 2), translateX(WINDOW_WIDTH / 2 - 10), "Press SPACE/ENTER to Start");
    mvprintw(translateY(WINDOW_HEIGHT / 2 + 4), translateX(WINDOW_WIDTH / 2 - 18), "Red:S | Yellow:B | Magenta->Green: Buff");
}

/*
 * Purpose: Render the difficulty selection screen with highlighting.
 * Inputs: selectedButton - index of the active option.
 * Returns: Nothing.
 */
void Display::drawDifficultySelect(int selectedButton) {
    mvprintw(translateY(WINDOW_HEIGHT / 4), translateX(WINDOW_WIDTH / 2 - 8), "SELECT DIFFICULTY");
    
    const char* options[] = { "[ EASY ]", "[ MEDIUM ]", "[ HARD ]", "[ QUIT ]" };
    
    // Vertical layout because window width (35) is too small for 4 horizontal buttons
    for (int i = 0; i < 4; i++) {
        if (i == selectedButton) {
            attron(A_REVERSE);
        }
        // Center vertically around middle
        int y = WINDOW_HEIGHT / 2 - 2 + i * 2;
        int x = WINDOW_WIDTH / 2 - (strlen(options[i]) / 2);
        mvaddstr(translateY(y), translateX(x), options[i]);
        
        if (i == selectedButton) {
            attroff(A_REVERSE);
        }
    }
    
    mvprintw(translateY(WINDOW_HEIGHT - 2), translateX(2), "W/S or Arrows to select, ENTER confirm");
}

/*
 * Purpose: Render the full gameplay scene including HUD and entities.
 * Inputs: References to the player, entity containers, boss pointer, and stats.
 * Returns: Nothing.
 */
void Display::drawPlaying(const Player& player, 
                 const std::vector<std::unique_ptr<Bullet>>& bullets,
                 const std::vector<std::unique_ptr<Enemy>>& enemies,
                 const Boss* boss,
                 int score, 
                 int enemiesDefeated, 
                 int playerLives, 
                 int bulletDamage, 
                 Difficulty currentDifficulty, 
                 bool autoShoot) {
    // Draw player
    player.draw();
    
    // Draw bullets
    for (const auto& bullet : bullets) {
        bullet->draw();
    }
    
    // Draw enemies
    for (const auto& enemy : enemies) {
        enemy->draw();
    }

    // Draw boss (if active)
    if (boss) {
        boss->draw();
    }
    
    // Draw game information
    if (has_colors()) {
        attron(COLOR_PAIR(4));
    }
    int hudY = 1;
    mvprintw(translateY(hudY), translateX(2),  "SCORE: %d", score);
    mvprintw(translateY(hudY), translateX(18), "KILLS: %d", enemiesDefeated);
    mvprintw(translateY(hudY), translateX(32), "LIVES: %d", playerLives);
    mvprintw(translateY(hudY), translateX(42), "DMG: %d", bulletDamage);

    // Show difficulty
    switch (currentDifficulty) {
        case Difficulty::EASY:
           mvprintw(translateY(hudY + 1), translateX(2), "MODE: EASY");
           break;
        case Difficulty::MEDIUM:
            mvprintw(translateY(hudY + 1), translateX(2), "MODE: MEDIUM");
            break;
        case Difficulty::HARD:
            mvprintw(translateY(hudY + 1), translateX(2), "MODE: HARD");
            break;
    }

    // Show Boss HP text when Boss is present
    if (boss && !boss->isDead()) {
        mvprintw(translateY(hudY + 1), translateX(24), "BOSS HP: %d/%d", boss->getHealth(), boss->getMaxHealth());
    }

    mvprintw(translateY(WINDOW_HEIGHT - 1), translateX(2),
             "WASD Move | SPACE Auto-Shoot[%s] | Q Menu",
             autoShoot ? "ON" : "OFF");

    if (has_colors()) {
        attroff(COLOR_PAIR(4));
    }
}

/*
 * Purpose: Show the game-over prompt and instructions.
 * Inputs: None.
 * Returns: Nothing.
 */
void Display::drawGameOver() {
    mvprintw(translateY(WINDOW_HEIGHT / 2 - 2), translateX(WINDOW_WIDTH / 2 - 5), "GAME OVER");
    mvprintw(translateY(WINDOW_HEIGHT / 2 + 2), translateX(WINDOW_WIDTH / 2 - 2), "[ x ]");
    mvprintw(translateY(WINDOW_HEIGHT / 2 + 4), translateX(WINDOW_WIDTH / 2 - 12), "Press x to view stats");
}

/*
 * Purpose: Show the win screen once the boss is defeated.
 * Inputs: None.
 * Returns: Nothing.
 */
void Display::drawWin() {
    mvprintw(translateY(WINDOW_HEIGHT / 2 - 2), translateX(WINDOW_WIDTH / 2 - 4), "YOU WIN!");
    mvprintw(translateY(WINDOW_HEIGHT / 2),     translateX(WINDOW_WIDTH / 2 - 10), "Boss has been defeated");
    mvprintw(translateY(WINDOW_HEIGHT / 2 + 3), translateX(WINDOW_WIDTH / 2 - 2), "[ x ]");
    mvprintw(translateY(WINDOW_HEIGHT / 2 + 5), translateX(WINDOW_WIDTH / 2 - 16), "Press x to view stats (STATS_VIEW)");
}

/*
 * Purpose: Render the stats/leaderboard screen with navigation buttons.
 * Inputs: currentDifficulty - active difficulty, leaderboard - stored scores,
 *         score - latest score, selectedButton - which footer button is active.
 * Returns: Nothing.
 */
void Display::drawStats(Difficulty currentDifficulty, const std::vector<int>& leaderboard, int score, int selectedButton) {
    // Show difficulty
    switch (currentDifficulty) {
        case Difficulty::EASY:
           mvprintw(translateY(1), translateX(2), "MODE: EASY");
           break;
        case Difficulty::MEDIUM:
            mvprintw(translateY(1), translateX(2), "MODE: MEDIUM");
            break;
        case Difficulty::HARD:
            mvprintw(translateY(1), translateX(2), "MODE: HARD");
            break;
    }

    // Draw Leaderboard Area (Top 2/3)
    mvprintw(translateY(2), translateX(WINDOW_WIDTH / 2 - 10), "LEADERBOARD (TOP 10)");
    
    // Draw table header
    mvprintw(translateY(4), translateX(WINDOW_WIDTH / 2 - 10), "RANK   SCORE");
    mvprintw(translateY(5), translateX(WINDOW_WIDTH / 2 - 10), "----   -----");
    
    // Draw entries
    for(size_t i = 0; i < leaderboard.size() && i < 10; i++) {
        mvprintw(translateY(6 + i), translateX(WINDOW_WIDTH / 2 - 10), "#%-2lu    %d", i + 1, leaderboard[i]);
    }
    
    // Draw Current Score Box
    int boxY = WINDOW_HEIGHT - 8;
    int boxX = WINDOW_WIDTH / 2 - 12;
    int boxW = 24;
    int boxH = 4;
    
    // Draw box frame
    mvhline(translateY(boxY), translateX(boxX), '-', boxW);
    mvhline(translateY(boxY + boxH), translateX(boxX), '-', boxW);
    mvvline(translateY(boxY), translateX(boxX), '|', boxH);
    mvvline(translateY(boxY), translateX(boxX + boxW), '|', boxH);
    mvaddch(translateY(boxY), translateX(boxX), '+');
    mvaddch(translateY(boxY), translateX(boxX + boxW), '+');
    mvaddch(translateY(boxY + boxH), translateX(boxX), '+');
    mvaddch(translateY(boxY + boxH), translateX(boxX + boxW), '+');
    
    mvprintw(translateY(boxY + 2), translateX(boxX + 2), "YOUR SCORE: %d", score);
    
    // Buttons: [ BACK ]  [ CLEAN ]
    const char* btnBack = "[ BACK ]";
    const char* btnClean = "[ CLEAN ]";
    
    int buttonY = WINDOW_HEIGHT - 2;
    int centerX = WINDOW_WIDTH / 2;
    int spacing = 10; // space between buttons center-to-center
    
    // Draw BACK button (index 0)
    if (selectedButton == 0) attron(A_REVERSE);
    mvaddstr(translateY(buttonY), translateX(centerX - spacing / 2 - 4), btnBack);
    if (selectedButton == 0) attroff(A_REVERSE);
    
    // Draw CLEAN button (index 1)
    if (selectedButton == 1) attron(A_REVERSE);
    mvaddstr(translateY(buttonY), translateX(centerX + spacing / 2 - 4), btnClean);
    if (selectedButton == 1) attroff(A_REVERSE);
}

