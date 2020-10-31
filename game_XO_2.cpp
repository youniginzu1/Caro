/*Game cờ caro-----------------------------------------
Luật chơi:
-Có một bảng 20*20 gồm các ô trắng
-Người chơi dùng chuột phải để đánh quân cờ(mặc định quân X)
-Mặc định người chơi đi trước
-Khi người chơi đạt được năm ô liên tiếp mà không bị chặn cả 2 đầu thì người chơi sẽ chiến thắng
-Nếu máy đạt được chuỗi đó thì người chơi thua
-git ngày 31/10/2020
*/

/*Khai báo thư viện----------------------------------*/

#include <iostream>
#include <ctime>           // Sử dụng cho hàm rand()
#include <vector>          // Lưu vị trí ảnh
#include <string>          // Lưu các đường dẫn
#include <SDL.h>           // Tạo cửa sổ và vẽ vào cửa sổ
#include <SDL_image.h>     // Load ảnh
#include <SDL_ttf.h>       // Viết chữ lên màn hình
#include <sstream>         // Xây dựng chuỗi từ số nguyên
using namespace std;


/*Hằng ----------------------------------------------*/

const int X_VALUE         = 1;        // Giá trị của ô X trong bảng
const int O_VALUE         = -1;       // Giá trị của ô O trong bảng
const int HIDDEN_VALUE    = 0;        // Giá trị của ô ẩn trong bảng

const int SCREEN_WIDTH    = 30;       // Chiều rộng gốc của một ô trong bảng
const int SCREEN_HEIGHT   = 30;       // Chiều cao gốc của một ô trong bảng

const int DEFAULT_NUM_COLS= 20;       // Số ô mặc định theo chiều ngang
const int DEFAULT_NUM_ROWS= 20;       // Số ô mặc định theo chiều dọc

const string SCREEN_TITLE    = "Cờ Caro" ;        // Tiêu đề của trò chơi
const string IMAGE_PATH      = "caro_image.png";  // Đường dẫn của ảnh
const string TEXT_PATH_1     = "caro_font_1.ttf"; // Đường dẫn đến phông chữ 1
const string TEXT_PATH_2     = "caro_font_2.ttf"; // Đường dẫn đến phông chữ 2

const int PAUSE_TIME         = 200;      // Thời gian dừng của mỗi nước đi

int win_human = 0;   // Tỉ số sau mỗi ván đấu của người
int win_boss = 0;    // Tỉ số sau mỗi ván đấu của máy


/*Kiểu dữ liệu mới---------------------------------------*/

struct Cell  // Tọa độ x, y với gốc O(0,0) nằm góc trên bên trái, trục Ox nằm ngang, trục Oy nằm dọc
{
    int x, y, id; // id là biến đánh dấu
    Cell()
    {
        x = 0;
        y = 0;
        id = 1;
    }
    Cell(int x_, int y_, int id_)
    {
        x = x_;
        y = y_;
        id = id_;
    }
    Cell(int x_, int y_)
    {
        x = x_;
        y = y_;
        id = 1;
    }
    void operator = (Cell p)
    {
        x = p.x;
        y = p.y;
        id = p.id;
    }
};

enum GameState      // Trạng thái của trò chơi
{
    GAME_PLAYING,   // Đang chơi
    GAME_WON,       // Đã thắng
    GAME_LOST,      // Đã thua
};

enum ImageType
{
    HIDDEN_IMAGE,   // Ảnh ô ẩn
    X_IMAGE,        // Ảnh ô X
    O_IMAGE,        // Ảnh ô O
    X_IMAGE_WON,    // Ảnh ô X khi chiến thắng
    O_IMAGE_WON,    // Ảnh ô O khi chiến thắng
    TOTAL_IMAGE     // Tổng số ảnh
};

struct Game
{
    int CellTable[DEFAULT_NUM_ROWS][DEFAULT_NUM_COLS]; // Mảng 2 chiều là bảng của trò chơi
    Cell CellRange[5]; // Mảng chứa các ô chiến thắng
    GameState state;   // Trạng thái của một game
};

struct Graphic
{
    SDL_Window* window;          // Cửa sổ hiển thị
    SDL_Renderer* renderer;      // Vẽ lên cửa sổ
    SDL_Texture* caroTexture;    // Load ảnh
    SDL_Texture* caroTexture1;   // Load text lên màn hình
    TTF_Font* font;              // Load text
    vector <SDL_Rect> imageRect; // Lưu vị trí ảnh
};


/* Các hàm của bàn game ----------------------------*/

// Khởi tạo SDL, SDL_Image, tạo cửa sổ, load ảnh, ...  Trả về false nếu khởi tạo không thành công
bool initGraphic(Graphic &g);

// Load ảnh từ đường dẫn path, trả về ảnh biểu diễn dưới dạng SDL_Texture
SDL_Texture* createTexture(SDL_Renderer *renderer, const string &path);

// Huỷ khởi tạo những gì đã dùng từ SDL và SDL_Image
void finalizeGraphic(Graphic &g);

// Hiển thị cửa sổ thông báo lỗi
void err(const string &m);

// Lưu vị trí các hình trong ảnh vào tham số rects
void initimageRects(vector <SDL_Rect> &rects);

// Khởi tạo game:bảng game, trang thái chơi
void initGame(Game &game);

// Hiển thị các quân cờ lên trên màn hình
void displayGame(Game &game, Graphic &g);

// Máy xử lí và trả về một tọa độ là nước đi thích hợp
Cell bossPlay(int CellTable[][DEFAULT_NUM_COLS]);

// Kiểm tra xem có ai chiến thắng không
int whoWin(Game &Game);

// Cập nhập game:nước đi của người và máy dần được cập nhập
void updateGame(Game &game, const SDL_Event &event, Graphic &g);

// Thực hiện các công việc của vẽ chữ
void drawText(string text, Graphic& g, int x, int y);


/* Các hàm của boss --------------------------------*/

// Hàm trả về đường đi của boss
Cell pathBoss(int CellTable[][DEFAULT_NUM_COLS], int id);

// Hàm kiểm tra xe boss có thắng được không
Cell checkWin(int CellTable[][DEFAULT_NUM_COLS], int id);

// Kiểm tra đường đi theo số quân cờ (VD: nước 3, nước 4..)
Cell checkPath(int CellTable[][DEFAULT_NUM_COLS], int id, int n);

// Thực hiện trạng thái phòng thủ
Cell defense(int CellTable[][DEFAULT_NUM_COLS], int id);

// Thực hiện trạng thái tấn công
Cell attack(int CellTable[][DEFAULT_NUM_COLS], int id);


/* Định nghĩa các hàm---------------------------*/

int main(int argc, char* argv[])
{
    srand(time(0)); // Khởi tạo seed cho hàm rand
    bool again = true;
    while(again)
    {
        Graphic graphic;
        if (!initGraphic(graphic))
        {
            finalizeGraphic(graphic);
            return EXIT_FAILURE;
        }
        Game game;
        initGame(game);
        bool quit = false;
        while(!quit)
        {
            displayGame(game, graphic);
            SDL_Event event;
            while(SDL_PollEvent(&event) != 0)
            {
                if(event.type == SDL_QUIT)
                {
                    quit = true;
                    again = false;
                    break;
                }
                if(game.state != GAME_PLAYING && event.type == SDL_KEYDOWN and event.key.keysym.sym == SDLK_RETURN)
                {
                    quit = true;
                    break;
                }
                updateGame(game, event, graphic);
            }
        }
        finalizeGraphic(graphic);
    }
    return EXIT_SUCCESS;
}


bool initGraphic(Graphic &g)
{
    g.window = NULL;
    g.renderer = NULL;
    g.caroTexture = NULL;
    g.caroTexture1 = NULL;
    g.font = NULL;


    int sdlFlags = SDL_INIT_VIDEO;  // Dùng SDL để hiển thị hình ảnh
    if (SDL_Init(sdlFlags) != 0)
    {
        err("SDL could not initialize!");
        return false;
    }

    if(TTF_Init() < 0)            // Khởi tạo thư viện ttf
    {
        err("SDL_TTF could not initialize!");
        return false;
    }

    int imgFlags = IMG_INIT_PNG;    // Dùng SDL_Image để load ảnh png
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        err("SDL_Image could not initialize!");
        return false;
    }

    g.window = SDL_CreateWindow(SCREEN_TITLE.c_str(),          // Tiêu đề của cửa sổ
                                SDL_WINDOWPOS_UNDEFINED,       // x
                                SDL_WINDOWPOS_UNDEFINED,       // y
                                DEFAULT_NUM_COLS*SCREEN_WIDTH, // Chiểu rộng của cửa sổ
                                DEFAULT_NUM_ROWS*SCREEN_HEIGHT,// Chiều cao của cửa sổ
                                SDL_WINDOW_SHOWN);             // Hiển thị ra
    if (g.window == NULL)
    {
        err("Window could not be created!");
        return false;
    }

    // Vẽ lên cửa sổ
    g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_ACCELERATED);
    if (g.renderer == NULL)
    {
        err("Renderer could not be created!");
        return false;
    }

    // Load ảnh
    g.caroTexture = createTexture(g.renderer, IMAGE_PATH);
    if (g.caroTexture == NULL)
    {
        err("Unable to create texture from " + IMAGE_PATH + "!");
        return false;
    }

    // Khởi tạo các vị trí của ảnh
    initimageRects(g.imageRect);
    return true;
}

SDL_Texture* createTexture(SDL_Renderer *renderer, const string &path)
 {
    SDL_Surface *surface = IMG_Load(path.c_str());
    if (surface == NULL)
    {
        err("Unable to load image " + path + "!");
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void finalizeGraphic(Graphic &g)
{
    SDL_DestroyTexture(g.caroTexture);
    SDL_DestroyTexture(g.caroTexture1);
    SDL_DestroyRenderer(g.renderer);
    SDL_DestroyWindow(g.window);

    IMG_Quit();
    SDL_Quit();
    TTF_Quit();
}

void initimageRects(vector <SDL_Rect> &rects)
{
    for(int i=0;i < TOTAL_IMAGE;i++)
    {
        ImageType type = (ImageType) i;
        SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        if(type >= HIDDEN_IMAGE && type <= O_IMAGE_WON)
        {
            rect.x = i*SCREEN_WIDTH;
        }
        rects.push_back(rect);
    }
}
void initGame(Game &game)
{
    for(int i=0;i < DEFAULT_NUM_ROWS;i++)
    {
        for(int j=0;j < DEFAULT_NUM_COLS;j++)
        {
            game.CellTable[i][j] = HIDDEN_VALUE;
        }
    }
    game.state = GAME_PLAYING;
}

void drawText(string text, Graphic& g, int x, int y)
{
    if(text == "YOU WIN!" || text == "YOU LOSE!")
    {
        g.font = TTF_OpenFont(TEXT_PATH_1.c_str(), 70);
    }
    if(text == "Press Enter To Play Again")
    {
        g.font = TTF_OpenFont(TEXT_PATH_2.c_str(), 20);
    }
    if(text == "Score ->")
    {
        g.font = TTF_OpenFont(TEXT_PATH_2.c_str(), 20);
        string score1, score2;
        ostringstream convert1,convert2;
        convert1 << win_human;
        convert2 << win_boss;
        score1 = convert1.str();
        score2 = convert2.str();
        text = text + "Human "+score1 + ":" + score2 + " Boss";//Score ->Human 0:0 Boss
    }

    SDL_Color fg = {255, 0, 0};

    SDL_Surface* surface = TTF_RenderText_Solid(g.font, text.c_str(), fg);
    g.caroTexture1 = SDL_CreateTextureFromSurface(g.renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect destRect;
    SDL_Rect srcRect;

    TTF_SizeText(g.font, text.c_str(), &srcRect.w, &srcRect.h);

    srcRect.x = 0;
    srcRect.y = 0;

    destRect.x = x;
    destRect.y = y;

    destRect.w = srcRect.w;
    destRect.h = srcRect.h;

    SDL_RenderCopy(g.renderer, g.caroTexture1, &srcRect, &destRect);
}

void displayGame(Game &game, Graphic &g)
{
    SDL_Rect srcRect;
    SDL_Rect destRect;
    if(game.state == GAME_WON || game.state == GAME_LOST)
    {
        for(int i=0;i<=4;i++)
        {
            // Vị trí của ô cần vẽ lên màn hình
            destRect = {game.CellRange[i].y*SCREEN_WIDTH,
                        game.CellRange[i].x*SCREEN_HEIGHT,
                        SCREEN_WIDTH,
                        SCREEN_HEIGHT};

            // Vị trí lấy từ file ảnh để vẽ
            if(game.state == GAME_WON)
            {
                srcRect = g.imageRect[X_IMAGE_WON];
            }
            else
            {
                srcRect = g.imageRect[O_IMAGE_WON];
            }
            // Vẽ lên màn hình
            SDL_RenderCopy(g.renderer, g.caroTexture, &srcRect,
                           &destRect);
        }
        if(game.state == GAME_WON)
        {
            drawText("YOU WIN!", g,
                    ((DEFAULT_NUM_COLS*SCREEN_WIDTH)/4)+20,
                    (DEFAULT_NUM_ROWS*SCREEN_HEIGHT)/2);
        }
        if(game.state == GAME_LOST)
        {
            drawText("YOU LOSE!", g,
                    (DEFAULT_NUM_COLS*SCREEN_WIDTH)/5,
                    (DEFAULT_NUM_ROWS*SCREEN_HEIGHT)/2);
        }
        drawText("Press Enter To Play Again", g,
                (DEFAULT_NUM_COLS*SCREEN_WIDTH)/4,
                (DEFAULT_NUM_ROWS*SCREEN_HEIGHT)/2+100);
        drawText("Score ->", g, 30, 40);
        // Hiển thì những gì đã vẽ lên màn hình
        SDL_RenderPresent(g.renderer);
        return;
    }
    SDL_RenderClear(g.renderer);      // Xoá những gì đã vẽ
    for(int i=0;i < DEFAULT_NUM_ROWS;i++)
    {
        for(int j=0;j < DEFAULT_NUM_COLS;j++)
        {
            // Vị trí của ô cần vẽ lên màn hình
            destRect = {j*SCREEN_WIDTH, i*SCREEN_HEIGHT,
                          SCREEN_WIDTH, SCREEN_HEIGHT};

            // Vị trí lấy từ file ảnh để vẽ
            if(game.CellTable[i][j] == X_VALUE)
            {
                srcRect = g.imageRect[X_IMAGE];
            }
            if(game.CellTable[i][j] == HIDDEN_VALUE)
            {
                srcRect = g.imageRect[HIDDEN_IMAGE];
            }
            if(game.CellTable[i][j] == O_VALUE)
            {
                srcRect = g.imageRect[O_IMAGE];
            }
            // Vẽ lên màn hình
            SDL_RenderCopy(g.renderer, g.caroTexture, &srcRect,
                           &destRect);
        }
    }
    // Hiển thì những gì đã vẽ lên màn hình
    SDL_RenderPresent(g.renderer);
}


void updateGame(Game &game, const SDL_Event &event, Graphic& g)
{
    if(game.state != GAME_PLAYING)
    {
        return;
    }
    if(event.type != SDL_MOUSEBUTTONDOWN)
    {
        return;
    }
    SDL_MouseButtonEvent mouse = event.button;// Sự kiện click chuột
    int row = mouse.y/SCREEN_HEIGHT;
    int col = mouse.x/SCREEN_WIDTH;
    if(game.CellTable[row][col] != HIDDEN_VALUE)
    {
        return;
    }
    if(mouse.button == SDL_BUTTON_LEFT)
    {
        game.CellTable[row][col] = X_VALUE;
    }
    int winner = whoWin(game);
    if(winner == X_VALUE)
    {
        game.state = GAME_WON;
        win_human++;
        return;
    }
    displayGame(game, g);
    //Bắt đầu lượt chơi của máy---------

    SDL_Delay(PAUSE_TIME);
    Cell cell = bossPlay(game.CellTable);
    game.CellTable[cell.x][cell.y] = O_VALUE;
    winner = whoWin(game);
    if(winner == O_VALUE)
    {
        game.state = GAME_LOST;
        win_boss++;
    }
}
int whoWin(Game &game)
{
    bool win = false;
    for(int i=0;i < DEFAULT_NUM_ROWS;i++)
    {
        for(int j=0;j < DEFAULT_NUM_COLS;j++)
        {
            if(game.CellTable[i][j] == HIDDEN_VALUE) continue;

            // Kiểm tra hàng dọc
            if((game.CellTable[i+1][j] == game.CellTable[i][j]) &&
               (game.CellTable[i+2][j] == game.CellTable[i][j]) &&
               (game.CellTable[i+3][j] == game.CellTable[i][j]) &&
               (game.CellTable[i+4][j] == game.CellTable[i][j]))
            {
                if((game.CellTable[i-1][j] == HIDDEN_VALUE) ||
                   (game.CellTable[i+5][j] == HIDDEN_VALUE))
                {
                    win = true;
                }
                if(win)
                {
                    // Lưu các vị trí win vào một mảng
                    for(int k=0;k <= 4;k++)
                    {
                        game.CellRange[k] = Cell(i+k,j);
                    }
                    return game.CellTable[i][j];
                }
            }

            //Kiểm tra hàng ngang
            if((game.CellTable[i][j+1] == game.CellTable[i][j]) &&
               (game.CellTable[i][j+2] == game.CellTable[i][j]) &&
               (game.CellTable[i][j+3] == game.CellTable[i][j]) &&
               (game.CellTable[i][j+4] == game.CellTable[i][j]))
            {
                if((game.CellTable[i][j-1] == HIDDEN_VALUE) ||
                   (game.CellTable[i][j+5] == HIDDEN_VALUE))
                {
                    win = true;
                }
                if(win)
                {
                    // Lưu các vị trí win vào một mảng
                    for(int k=0;k <=4;k++)
                    {
                        game.CellRange[k] = Cell(i,j+k);
                    }
                    return game.CellTable[i][j];
                }
            }

            // Kiểm tra đường chéo trên
            if((game.CellTable[i-1][j+1] == game.CellTable[i][j]) &&
               (game.CellTable[i-2][j+2] == game.CellTable[i][j]) &&
               (game.CellTable[i-3][j+3] == game.CellTable[i][j]) &&
               (game.CellTable[i-4][j+4] == game.CellTable[i][j]))
            {
                if((game.CellTable[i+1][j-1] == HIDDEN_VALUE) ||
                   (game.CellTable[i-5][j+5] == HIDDEN_VALUE))
                {
                    win = true;
                }
                if(win)
                {
                    // Lưu các vị trí win vào một mảng
                    for(int k=0;k <=4;k++)
                    {
                        game.CellRange[k] = Cell(i-k,j+k);
                    }
                    return game.CellTable[i][j];
                }
            }

            // Kiểm tra đường chéo dưới
            if((game.CellTable[i+1][j+1] == game.CellTable[i][j]) &&
               (game.CellTable[i+2][j+2] == game.CellTable[i][j]) &&
               (game.CellTable[i+3][j+3] == game.CellTable[i][j]) &&
               (game.CellTable[i+4][j+4] == game.CellTable[i][j]))
            {
                if((game.CellTable[i-1][j-1] == HIDDEN_VALUE) ||
                   (game.CellTable[i+5][j+5] == HIDDEN_VALUE))
                {
                    win = true;
                }
                if(win)
                {
                    // Lưu các vị trí win vào một mảng
                    for(int k=0;k <=4;k++)
                    {
                        game.CellRange[k] = Cell(i+k,j+k);
                    }
                    return game.CellTable[i][j];
                }
            }
        }
    }
    return 0;
}

void err(const string &m)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", m.c_str(), NULL);
}

Cell bossPlay(int CellTable[][DEFAULT_NUM_COLS])
{
    return pathBoss(CellTable, O_VALUE);
}

/* Khu vực định nghĩa hàm boss-------------------------*/

Cell pathBoss(int CellTable[][DEFAULT_NUM_COLS], int id)
{
    Cell cell = checkWin(CellTable, id); // Đánh win
    if(cell.x != -1 && cell.y != -1) return cell;
    else
    {
        cell = defense(CellTable, id);  // Phòng thủ
        if(cell.x != -1 && cell.y != -1) return cell;
        cell = checkPath(CellTable, id, 3);
        if(cell.x != -1 && cell.y != -1 && cell.id == 0) return cell;
        cell = checkPath(CellTable, -id, 3);
        if(cell.x != -1 && cell.y != -1 && cell.id == 0) return cell;
        return attack(CellTable, id);   // Tấn công
    }
}

Cell checkWin(int CellTable[][DEFAULT_NUM_COLS], int id)
{
    return checkPath(CellTable, id, 4);
}

Cell checkPath(int CellTable[][DEFAULT_NUM_COLS], int id, int n)
{
    int check_6h = 1, check_3h = 1, check_1h = 1, check_5h = 1;
    Cell twoFist[8]; // Mảng lưu các vị trị hai đầu của nước đi
    int dem = 0;     // Biến đếm của mảng trên
    for(int i=0;i < DEFAULT_NUM_ROWS;i++)
    {
        for(int j=0;j < DEFAULT_NUM_COLS;j++)
        {
            if(CellTable[i][j] != id) continue;
            check_6h = 1, check_3h = 1, check_1h = 1, check_5h = 1;

            // Đếm số ô liên tiếp của nước đi n
            for(int k=1;k < n;k++)
            {
                if(CellTable[i+k][j] == id) check_6h++;
                if(CellTable[i][j+k] == id) check_3h++;
                if(CellTable[i-k][j+k] == id) check_1h++;
                if(CellTable[i+k][j+k] == id) check_5h++;
            }

            if(check_6h == n)
            {
                // Trả về nước đi 3 liên tiếp 2 đầu ko chặn
                if(n == 3 && (CellTable[i-1][j] == HIDDEN_VALUE) && (CellTable[i+n][j] == HIDDEN_VALUE))
                {
                    return Cell(i-1, j, 0);
                }
                if(CellTable[i-1][j] == HIDDEN_VALUE)
                {
                    twoFist[dem] = Cell(i-1, j);
                    dem++;
                }
                if(CellTable[i+n][j] == HIDDEN_VALUE)
                {
                    twoFist[dem] = Cell(i+n, j);
                    dem++;
                }
            }
            if(CellTable[i+n][j] == id  && check_6h == n-1)
            {
                // Trả về nước đi 3 không liên tiếp 2 đầu ko chặn
                if(n == 3 && (CellTable[i-1][j] == HIDDEN_VALUE) && (CellTable[i+n+1][j] == HIDDEN_VALUE))
                {
                    for(int index = 1; index <=3; index++)
                    {
                        if(CellTable[i+index][j] == HIDDEN_VALUE) return Cell(i+index, j, 0);
                    }
                }
                // Trả về nước đi 4 không liên tiếp có ít nhất 1 trong 2  đầu ko chặn
                if(n == 4 && ((CellTable[i-1][j] == HIDDEN_VALUE) || (CellTable[i+n+1][j] == HIDDEN_VALUE)))
                {
                    for(int index = 1; index <=4; index++)
                    {
                        if(CellTable[i+index][j] == HIDDEN_VALUE)
                        {
                            twoFist[dem] = Cell(i+index, j);
                            dem++;
                        }
                    }
                }
            }

            if(check_3h == n)
            {
                // Trả về nước đi 3 liên tiếp 2 đầu ko chặn
                if(n == 3 && (CellTable[i][j-1] == HIDDEN_VALUE) && (CellTable[i][j+n] == HIDDEN_VALUE))
                {
                    return Cell(i, j-1, 0);
                }
                if(CellTable[i][j-1] == HIDDEN_VALUE)
                {
                    twoFist[dem] = Cell(i, j-1);
                    dem++;
                }
                if(CellTable[i][j+n] == HIDDEN_VALUE)
                {
                    twoFist[dem] = Cell(i, j+n);
                    dem++;
                }
            }
            if(CellTable[i][j+n] == id && check_3h == n-1)
            {
                // Trả về nước đi 3 không liên tiếp 2 đầu ko chặn
                if(n == 3 && (CellTable[i][j-1] == HIDDEN_VALUE) && (CellTable[i][j+n+1] == HIDDEN_VALUE))
                {
                    for(int index = 1; index <=3; index++)
                    {
                        if(CellTable[i][j+index] == HIDDEN_VALUE) return Cell(i, j+index, 0);
                    }
                }
                // Trả về nước đi 4 không liên tiếp có ít nhất 1 trong 2  đầu ko chặn
                if(n == 4 && ((CellTable[i][j-1] == HIDDEN_VALUE) || (CellTable[i][j+n+1] == HIDDEN_VALUE)))
                {
                    for(int index = 1; index <=4; index++)
                    {
                        if(CellTable[i][j+index] == HIDDEN_VALUE)
                        {
                            twoFist[dem] = Cell(i, j+index);
                            dem++;
                        }
                    }
                }
            }

            if(check_1h == n)
            {
                // Trả về nước đi 3 liên tiếp 2 đầu ko chặn
                if(n == 3 && (CellTable[i+1][j-1] == HIDDEN_VALUE) && (CellTable[i-n][j+n] == HIDDEN_VALUE))
                {
                    return Cell(i+1, j-1, 0);
                }
                if(CellTable[i+1][j-1] == HIDDEN_VALUE)
                {
                    twoFist[dem] = Cell(i+1, j-1);
                    dem++;
                }
                if(CellTable[i-n][j+n] == HIDDEN_VALUE)
                {
                    twoFist[dem] = Cell(i-n, j+n);
                    dem++;
                }
            }
            if(CellTable[i-n][j+n] == id && check_1h == n-1)
            {
                // Trả về nước đi 3 không liên tiếp 2 đầu ko chặn
                if(n == 3 && (CellTable[i+1][j-1] == HIDDEN_VALUE) && (CellTable[i-n+1][j+n+1] == HIDDEN_VALUE))
                {
                    for(int index = 1; index <=3; index++)
                    {
                        if(CellTable[i-index][j+index] == HIDDEN_VALUE) return Cell(i-index, j+index, 0);
                    }
                }
                // Trả về nước đi 4 không liên tiếp có ít nhất 1 trong 2  đầu ko chặn
                if(n == 4 && ((CellTable[i+1][j-1] == HIDDEN_VALUE) || (CellTable[i-n+1][j+n+1] == HIDDEN_VALUE)))
                {
                    for(int index = 1; index <=4; index++)
                    {
                        if(CellTable[i-index][j+index] == HIDDEN_VALUE)
                        {
                            twoFist[dem] = Cell(i-index, j+index);
                            dem++;
                        }
                    }
                }
            }

            if(check_5h == n)
            {
                // Trả về nước đi 3 liên tiếp 2 đầu ko chặn
                if(n == 3 && (CellTable[i-1][j-1] == HIDDEN_VALUE) && (CellTable[i+n][j+n] == HIDDEN_VALUE))
                {
                    return Cell(i-1, j-1, 0);
                }
                if(CellTable[i-1][j-1] == HIDDEN_VALUE)
                {
                    twoFist[dem] = Cell(i-1, j-1);
                    dem++;
                }
                if(CellTable[i+n][j+n] == HIDDEN_VALUE)
                {
                    twoFist[dem] = Cell(i+n, j+n);
                    dem++;
                }
            }
            if(CellTable[i+n][j+n] == id && check_5h == n-1)
            {
                // Trả về nước đi 3 không liên tiếp 2 đầu ko chặn
                if(n == 3 && (CellTable[i-1][j-1] == HIDDEN_VALUE) && (CellTable[i+n+1][j+n+1] == HIDDEN_VALUE))
                {
                    for(int index = 1; index <=3; index++)
                    {
                        if(CellTable[i+index][j+index] == HIDDEN_VALUE) return Cell(i+index, j+index, 0);
                    }
                }
                // Trả về nước đi 4 không liên tiếp có ít nhất 1 trong 2  đầu ko chặn
                if(n == 4 && ((CellTable[i-1][j-1] == HIDDEN_VALUE) || (CellTable[i+n+1][j+n+1] == HIDDEN_VALUE)))
                {
                    for(int index = 1; index <=4; index++)
                    {
                        if(CellTable[i+index][j+index] == HIDDEN_VALUE)
                        {
                            twoFist[dem] = Cell(i+index, j+index);
                            dem++;
                        }
                    }
                }
            }

            if(dem > 0)
            {
                return twoFist[rand()%dem];
            }
        }
    }
    return Cell(-1,-1);
}

Cell defense(int CellTable[][DEFAULT_NUM_COLS], int id)
{
    return checkPath(CellTable, -id, 4);
}

Cell attack(int CellTable[][DEFAULT_NUM_COLS], int id)
{
    Cell cell = checkPath(CellTable, id, 3);     // Tấn công nước 3
    if(cell.x != -1 && cell.y != -1) return cell;
    cell = checkPath(CellTable, id, 2);          // Tấn công nước 2
    if(cell.x != -1 && cell.y != -1) return cell;
    cell = checkPath(CellTable, id, 1);          // Tấn công nước 1
    if(cell.x != -1 && cell.y != -1) return cell;
    for(int i=0;i < DEFAULT_NUM_ROWS;i++)
    {
        for(int j=0;j < DEFAULT_NUM_COLS;j++)
        {
            if(CellTable[i][j] == -id)
            {
                int a[2] = {-1, 1};
                return Cell(i+a[rand()%2], j+a[rand()%2]);             // Tấn công nước 0
            }
        }
    }
    return Cell(DEFAULT_NUM_ROWS/2, DEFAULT_NUM_COLS/2);
}
