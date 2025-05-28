#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;
const int GRID_SIZE = 9;
const int CELL_SIZE = SCREEN_WIDTH / GRID_SIZE;

int sudoku[9][9] = {};

bool fixed[9][9]; // Đánh dấu ô ban đầu không được thay đổi

#include <algorithm>
#include <random>
#include <ctime>

bool isSafe(int row, int col, int num) {
    for (int x = 0; x < 9; x++) {
        if (sudoku[row][x] == num || sudoku[x][col] == num)
            return false;
    }

    int startRow = row - row % 3, startCol = col - col % 3;
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (sudoku[startRow + y][startCol + x] == num)
                return false;
        }
    }

    return true;
}

bool fillSudoku(int row, int col) {
    if (row == 9) return true;
    if (col == 9) return fillSudoku(row + 1, 0);
    if (sudoku[row][col] != 0) return fillSudoku(row, col + 1);

    std::vector<int> numbers = { 1,2,3,4,5,6,7,8,9 };
    std::shuffle(numbers.begin(), numbers.end(), std::mt19937(std::random_device()()));

    for (int num : numbers) {
        if (isSafe(row, col, num)) {
            sudoku[row][col] = num;
            if (fillSudoku(row, col + 1))
                return true;
            sudoku[row][col] = 0;
        }
    }

    return false;
}

void removeCells(int blanks = 40) {
    std::srand(time(0));
    while (blanks > 0) {
        int row = rand() % 9;
        int col = rand() % 9;
        if (sudoku[row][col] != 0) {
            sudoku[row][col] = 0;
            blanks--;
        }
    }
}

void renderGrid(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Đen
    for (int i = 0; i <= GRID_SIZE; i++) {
        int thickness = (i % 3 == 0) ? 3 : 1;
        for (int t = 0; t < thickness; t++) {
            SDL_RenderDrawLine(renderer, i * CELL_SIZE + t, 0, i * CELL_SIZE + t, SCREEN_HEIGHT);
            SDL_RenderDrawLine(renderer, 0, i * CELL_SIZE + t, SCREEN_WIDTH, i * CELL_SIZE + t);
        }
    }
}

void renderNumbers(SDL_Renderer* renderer, TTF_Font* font) {
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            if (sudoku[y][x] != 0) {
                std::string num = std::to_string(sudoku[y][x]);

                // Màu khác nhau tùy thuộc vào ô cố định hay không
                SDL_Color color;
                if (fixed[y][x])
                    color = { 0, 0, 0 }; // Màu đen cho ô ban đầu
                else
                    color = { 0, 0, 255 }; // Màu xanh dương cho ô người chơi nhập

                SDL_Surface* surface = TTF_RenderText_Solid(font, num.c_str(), color);
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

                int textW, textH;
                SDL_QueryTexture(texture, NULL, NULL, &textW, &textH);
                SDL_Rect dest = {
                    x * CELL_SIZE + (CELL_SIZE - textW) / 2,
                    y * CELL_SIZE + (CELL_SIZE - textH) / 2,
                    textW,
                    textH
                };

                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
                SDL_FreeSurface(surface);
            }
        }
    }
}

void renderSelection(SDL_Renderer* renderer, int selectedX, int selectedY) {
    if (selectedX >= 0 && selectedY >= 0) {
        SDL_Rect cellRect = {
            selectedX * CELL_SIZE,
            selectedY * CELL_SIZE,
            CELL_SIZE,
            CELL_SIZE
        };
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Màu xám nhạt
        SDL_RenderFillRect(renderer, &cellRect);
    }
}

bool isValidSudoku() {
    // Kiểm tra từng hàng
    for (int y = 0; y < 9; y++) {
        bool seen[10] = {};
        for (int x = 0; x < 9; x++) {
            int val = sudoku[y][x];
            if (val < 1 || val > 9 || seen[val])
                return false;
            seen[val] = true;
        }
    }

    // Kiểm tra từng cột
    for (int x = 0; x < 9; x++) {
        bool seen[10] = {};
        for (int y = 0; y < 9; y++) {
            int val = sudoku[y][x];
            if (val < 1 || val > 9 || seen[val])
                return false;
            seen[val] = true;
        }
    }

    // Kiểm tra từng ô 3x3
    for (int blockY = 0; blockY < 3; blockY++) {
        for (int blockX = 0; blockX < 3; blockX++) {
            bool seen[10] = {};
            for (int dy = 0; dy < 3; dy++) {
                for (int dx = 0; dx < 3; dx++) {
                    int val = sudoku[blockY * 3 + dy][blockX * 3 + dx];
                    if (val < 1 || val > 9 || seen[val])
                        return false;
                    seen[val] = true;
                }
            }
        }
    }

    return true;
}


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Sudoku SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("font.ttf", 48);
    if (!font) {
        std::cerr << "Loi font\n";
        return -1;
    }

    fillSudoku(0, 0);     // Tạo bảng hoàn chỉnh đúng luật
    removeCells(40);      // Xóa 40 ô để tạo thử thách

    // Cập nhật lại mảng fixed
    for (int y = 0; y < 9; y++)
        for (int x = 0; x < 9; x++)
            fixed[y][x] = sudoku[y][x] != 0;

    bool quit = false;
    SDL_Event e;
    int selectedX = -1, selectedY = -1;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x = e.button.x / CELL_SIZE;
                int y = e.button.y / CELL_SIZE;
                if (!fixed[y][x]) {
                    selectedX = x;
                    selectedY = y;
                }
            }
            else if (e.type == SDL_KEYDOWN && selectedX >= 0 && selectedY >= 0) {
                if (e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym <= SDLK_9) {
                    sudoku[selectedY][selectedX] = e.key.keysym.sym - SDLK_0;
                }
                else if (e.key.keysym.sym == SDLK_BACKSPACE || e.key.keysym.sym == SDLK_DELETE) {
                    sudoku[selectedY][selectedX] = 0;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        renderSelection(renderer, selectedX, selectedY);  // Vẽ ô được chọn
        renderNumbers(renderer, font);                    // Vẽ số
        renderGrid(renderer);                             // Vẽ lưới

        SDL_RenderPresent(renderer);

        if (isValidSudoku()) {
            std::cout << "Ban da hoan thanh Sudoku dung luat!\n";
            SDL_Delay(3000); // Hiển thị kết quả trong 3 giây
            quit = true;
        }
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
