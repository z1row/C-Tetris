#include <stdlib.h> 
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

#define COLOR_BACK 23
int ROW=0,COL=0;
int theme=1;
int welc=1;

/**
 * Function draws welcome screen
 */
void welcome();


/**
 * Function draws main menu
 */
void main_menu();


/**
 * Function draws settings menu
 */
void settings_menu();


/**
 * Function draws themes menu
 */
void theme_menu();


/**
 * Function draws welcome screen settings
 */
void initial_menu();


/**
 * Function draws play menu
 */
void play_menu();


/**
 * Function draws end screen
 */
void end_menu();


/**
 * Function with game logic
 * @param level expresses the difficulty level of the game
*/
void game(int level);


/**
 * Function draws current gamefield
 * @param gamewin Window for output
 * @param field Game field 
*/
void PrintField(WINDOW* gamewin, int field[38][19]);


/**
 * Function rotates the shape by 90 degrees
 * @param Shape 2d array expresses shape
*/
void RotateShape(int Shape[3][3]);


/**
 * Function inserts new shape in game field
 * @param field Game field
 * @param Shape 2d array expresses shape
 * @param Y possition in row
 * @param X possition in col
*/
int InsertNewShape(int field[38][19], int Shape[3][3], int* Y, int* X);


/**
 * Function changes rotated shape in game field
 * @param field Game field
 * @param Shape 2d array expresses shape
 * @param Y possition in row
 * @param X possition in col
 * 
*/
int ChangeShape(int field[38][19], int Shape[3][3], const int Y, const int X);


/**
 * Function shifts shape one row down
 * @param field Game field
 * @param Shape 2d array expresses shape
 * @param Y possition in row
 * @param X possition in col
*/
int ShiftDown(int field[38][19], int Shape[3][3], int* Y, const int X);


/**
 * Function shifts shape left/right depends on the parameter 'side', '0' – left / '1' – right
 * @param field Game field
 * @param Shape 2d array expresses shape
 * @param Y possition in row
 * @param X possition in col
 * @param side direction of movement
*/
int ShiftSide(int field[38][19], int Shape[3][3], const int Y, int* X, const int side);


/**
 * Initial setting of parameters and terminal
 */
void init(){
    if (access("settings.txt", F_OK)==0){
        FILE *settings=fopen("settings.txt","r");
        fseek(settings,0,0);
        fscanf(settings,"%d%d",&theme,&welc);
        // char input=fgetc(settings);
        fclose(settings);
    }
    else{
        FILE *settings=fopen("settings.txt","w");
        fputs("1 1",settings);
        fclose(settings);
    }
    initscr();
    start_color();
    cbreak();
    noecho();
    curs_set(0);
    init_color(COLOR_BACK,106, 90, 205);
    init_pair(1,COLOR_WHITE, COLOR_BACK);
    init_pair(2,COLOR_BACK, COLOR_WHITE);
    init_pair(3,COLOR_WHITE,COLOR_BLACK);
    init_pair(4,COLOR_BLACK,COLOR_WHITE);
    init_pair(100,COLOR_GREEN,COLOR_GREEN);
    bkgd(COLOR_PAIR(theme));
    getmaxyx(stdscr,ROW,COL);
}


int main(int argc, char *argv[]){
    init();
    if( argc == 2 ) {
        if (argv[1][0]=='g' && argv[1][0]=='a' && argv[1][0]=='m' && argv[1][0]=='e'){
            play_menu();
        }
        else if (argv[1][0]=='g'){
            endwin();
            printf("Unknown argument '%s'. Seems you tried to use argument 'game'.\n",argv[1]);
            return 0;
        }
        else{
            endwin();
            printf("Unknown argument '%s'.\n",argv[1]);
            return 0;
        }
    }
    else if( argc > 2 ) {
        endwin();
        printf("Too much...  One argument expected!\n");
        return 0;
    }
    if (welc==1)
        welcome();
    main_menu();
    clear();
    refresh();
    endwin();
    return 0;
}

void game(int level){
    WINDOW* guide= newwin(10,26,ROW/2-20+7,COL/2-20+50);
    wbkgd(guide, COLOR_PAIR(theme));
    box(guide,0,0);
    mvwprintw(guide, 0, 26/2-6, "| CONTROLS |");
    mvwprintw(guide, 2, 4, "'a' to move left");
    mvwprintw(guide, 3, 4, "'d' to move right");
    mvwprintw(guide, 4, 4, "'s' to move down");
    mvwprintw(guide, 5, 4, "'w' to rotate");
    mvwprintw(guide, 6, 4, "'p' to pause game");
    mvwprintw(guide, 7, 4, "'ESC' to exit game");
    wrefresh(guide);

    WINDOW* next_shape=newwin(10,26,ROW/2-20+7,COL/2-20-37);
    wbkgd(next_shape, COLOR_PAIR(theme));
    box(next_shape,0,0);
    mvwprintw(next_shape, 0, 26/2-7, "| NEXT SHAPE |");
    wrefresh(next_shape);



    int Shapes[7][3][3]={
        {{0,0,0},{1,1,1},{0,0,0}},
        {{0,0,0},{1,1,1},{0,1,0}},
        {{0,0,0},{0,1,1},{1,1,0}},
        {{0,0,0},{1,1,0},{0,1,1}},
        {{0,0,0},{1,1,1},{1,0,0}},
        {{0,0,0},{1,1,1},{0,0,1}},
        {{0,0,0},{0,1,1},{0,1,1}}
    };
    WINDOW* gamewin= newwin(40,40,ROW/2-20+7,COL/2-20);
    int field[38][19];
    for (int i=0;i<38;i++)
        for (int j=0;j<19;j++)
            field[i][j]=0;
    wbkgd(gamewin, COLOR_PAIR(theme));
    box(gamewin,0,0);
    wmove(gamewin,1,1);
  
    nodelay(gamewin, TRUE);
    nodelay(stdscr,1);
    
        int next_r_Shape[3][3];
        int random=rand()%7;
        for (int i=0;i<3;i++)
            for (int j=0;j<3;j++)
                next_r_Shape[i][j]=Shapes[random][i][j];

    int res=1;
    int game=0;


    while (game==0){
        int Y=0,X=0;
        res=0;
        int r_Shape[3][3];
        for (int i=0;i<3;i++)
            for (int j=0;j<3;j++)
                r_Shape[i][j]=next_r_Shape[i][j];
        int random=rand()%7;
        for (int i=0;i<3;i++)
            for (int j=0;j<3;j++)
                next_r_Shape[i][j]=Shapes[random][i][j];
        mvwprintw(next_shape, 3, 1, "                        ");
        mvwprintw(next_shape, 4, 1, "                        ");
        mvwprintw(next_shape, 5, 1, "                        ");
        wrefresh(next_shape);
        for (int i=0;i<3;i++)
            for (int j=0,k=0;j<3;j++,k++)
                if (next_r_Shape[i][j]==1){
                    wattron(next_shape,COLOR_PAIR(theme+1));
                    mvwaddch(next_shape, 3+i, 10+k, ' ');
                    k++;
                    mvwaddch(next_shape, 3+i, 10+k, ' ');
                    wattron(next_shape,COLOR_PAIR(theme));
                }
                else{
                    mvwaddch(next_shape, 3+i, 10+k, ' ');
                    k++;
                    mvwaddch(next_shape, 3+i, 10+k, ' ');
                }
        wrefresh(next_shape);

        game=InsertNewShape(field,r_Shape,&Y,&X);
        PrintField(gamewin,field);
        while (res!=-1){
        
        
        wtimeout(gamewin,700-(80*level));
        switch(wgetch(gamewin)){
            case 's':
                res=ShiftDown(field,r_Shape,&Y,X);
                break;
            case 'd':
                if (X<16)
                    ShiftSide(field,r_Shape,Y,&X,1);
                if (X==16){
                    if (r_Shape[0][2]==0 && r_Shape[1][2]==0 && r_Shape[2][2]==0)
                        ShiftSide(field,r_Shape,Y,&X,1);
                    break;
                }
                break;
            case 'a':
                if (X>0)
                    ShiftSide(field,r_Shape,Y,&X,0);
                else if (X==0){
                    if (r_Shape[0][0]==0 && r_Shape[1][0]==0 && r_Shape[2][0]==0)
                        ShiftSide(field,r_Shape,Y,&X,0);
                    break;
                }
                break;
            case 'w':
                if (r_Shape[0][0] != 0 || r_Shape[0][1] != 0 || r_Shape[0][2] != 0 || 
                    r_Shape[1][0] != 0 || r_Shape[1][1] != 1 || r_Shape[1][2] != 1 ||
                    r_Shape[2][0] != 0 || r_Shape[2][1] != 1 || r_Shape[2][2] != 1 ){
                    ChangeShape(field,r_Shape,Y,X);
                }
                break;
            case 'p':
                while (wgetch(gamewin)!='p'){
                    mvwprintw(stdscr, ROW/2-20+7-1, COL/2-5, "< PAUSED >");
                    wrefresh(stdscr);
                }
                mvwprintw(stdscr, ROW/2-20+7-1, COL/2-5, "          ");
                wrefresh(stdscr);
                break;
            case 27:
                return;
                break;    
            default:
                break;
            }
        res=ShiftDown(field,r_Shape,&Y,X);
        PrintField(gamewin,field); 
        }
    }
    end_menu();
}

void PrintField(WINDOW* gamewin, int field[38][19]){
    for (int i=0;i<38;i++)
            for (int j=0;j<19;j++)
                if (field[i][j]==1){
                    wattron(gamewin, COLOR_PAIR(theme+1));
                    mvwaddch(gamewin,i+1,1+2*j,' ');
                    mvwaddch(gamewin,i+1,2+2*j,' ');
                    wattron(gamewin, COLOR_PAIR(theme));
                }
                else{
                    mvwaddch(gamewin,i+1,1+2*j,' ');
                    mvwaddch(gamewin,i+1,2+2*j,' ');
                }
    wrefresh(gamewin);
}

void RotateShape(int Shape[3][3]){
    int temp_Shape[3][3];
    for (int i=0;i<3;i++)
        for (int j=0;j<3;j++)
            temp_Shape[i][j]=Shape[i][j];
    for(int i=0;i<3;i++){
        for(int j=0,k=3-1;j<3;j++,k--){
                Shape[i][j]=temp_Shape[k][i];
        }
    }
}

int InsertNewShape(int field[38][19], int Shape[3][3], int* Y, int* X){
    int temp_field[38][19];
    for (int i=0;i<38;i++)
        for (int j=0;j<19;j++){
            temp_field[i][j]=field[i][j];
    }
    int pos=rand()%16;
    *X=pos;
    *Y=0;
    for (int i=0,k=0;i<3;i++,k++){
        if (Shape[i][0]==0 && Shape[i][1]==0 && Shape[i][2]==0)
            k--;
        else 
            for (int j=0;j<3;j++)
                if (Shape[i][j]==1){
                    if (temp_field[k][j+pos]==1)
                        return -1;
                    temp_field[k][j+pos]=Shape[i][j];
                }
    }
    for (int i=0;i<38;i++)
        for (int j=0;j<19;j++)
            field[i][j]=temp_field[i][j];
    return 0;
}

int ChangeShape(int field[38][19], int Shape[3][3], const int Y, const int X){
    int temp_field[38][19];
    for (int i=0;i<38;i++)
        for (int j=0;j<19;j++){
            temp_field[i][j]=field[i][j];
    }
    int old_Shape[3][3];
    for (int i=0;i<3;i++)
        for (int j=0;j<3;j++)
            old_Shape[i][j]=Shape[i][j];
    RotateShape(Shape);
    // Clearing old Shape
    for (int i=0,k=0;i<3;i++,k++){
        if (old_Shape[i][0]==0 && old_Shape[i][1]==0 && old_Shape[i][2]==0)
            k--;
        else 
            for (int j=0;j<3;j++)
                if (old_Shape[i][j]==1){
                    temp_field[k+Y][j+X]=0;
                }
    }
    
    for (int i=0,k=0;i<3;i++,k++){
        if (Shape[i][0]==0 && Shape[i][1]==0 && Shape[i][2]==0)
            k--;
        else 
            for (int j=0;j<3;j++)
                if (Shape[i][j]==1){
                    if (temp_field[k+Y][j+X]==1)
                        return -1;
                    temp_field[k+Y][j+X]=Shape[i][j];
                }
    }

    for (int i=0;i<38;i++)
        for (int j=0;j<19;j++)
            field[i][j]=temp_field[i][j];
    return 0;
}

int ShiftDown(int field[38][19], int Shape[3][3], int* Y, const int X){
    int temp_field[38][19];
    for (int i=0;i<38;i++)
        for (int j=0;j<19;j++){
            temp_field[i][j]=field[i][j];
    }
    // Clearing old Shape
    for (int i=0,k=0;i<3;i++,k++)
        if (Shape[i][0]==0 && Shape[i][1]==0 && Shape[i][2]==0)
            k--;
        else 
            for (int j=0;j<3;j++)
                if (Shape[i][j]==1)
                    temp_field[k+*Y][j+X]=0;
    //
    *Y=*Y+1;
    if (Shape)
    for (int i=0,k=0;i<3;i++,k++){
        if (Shape[i][0]==0 && Shape[i][1]==0 && Shape[i][2]==0)
            k--;
        else 
            for (int j=0;j<3;j++)
                if (Shape[i][j]==1){
                    if (temp_field[k+*Y][j+X]==1 || k+*Y>37){
                        *Y=*Y-1;
                        return -1;
                    }
                    temp_field[k+*Y][j+X]=Shape[i][j];
                }
    }
    //
    for (int i=0;i<38;i++)
        for (int j=0;j<19;j++){
            field[i][j]=temp_field[i][j];
        }
    return 0;
}

int ShiftSide(int field[38][19], int Shape[3][3], const int Y, int* X, const int side){
    int temp_field[38][19];
    for (int i=0;i<38;i++)
        for (int j=0;j<19;j++){
            temp_field[i][j]=field[i][j];
    }
    // Clearing old Shape
    for (int i=0,k=0;i<3;i++,k++)
        if (Shape[i][0]==0 && Shape[i][1]==0 && Shape[i][2]==0)
            k--;
        else 
            for (int j=0;j<3;j++)
                if (Shape[i][j]==1)
                    temp_field[k+Y][j+*X]=0;
    //
    if (side==0)
        *X=*X-1;
    if (side==1)
        *X=*X+1;
    for (int i=0,k=0;i<3;i++,k++){
        if (Shape[i][0]==0 && Shape[i][1]==0 && Shape[i][2]==0)
            k--;
        else 
            for (int j=0;j<3;j++)
                if (Shape[i][j]==1){
                    if (temp_field[k+Y][j+*X]==1){
                        if (side==0)
                            *X=*X+1;
                        if (side==1)
                            *X=*X-1;
                        return -1;
                    }
                    temp_field[k+Y][j+*X]=Shape[i][j];
                }
    }
    //
    for (int i=0;i<38;i++)
        for (int j=0;j<19;j++){
            field[i][j]=temp_field[i][j];
        }
    return 0;
}



int draw_Shape(const int row, const int col, const char Shape[row][col], int shift){
    int size=0;
    for (size=0;size<8;size++)
        if (Shape[0][size]=='\0')
            break;
    for (int i=0;i<row;i++)
        for (int j=0;j<size;j++)
            if (Shape[i][j]=='A'){
            attron(COLOR_PAIR(theme+1));
            mvprintw(10+i,(COL/2-33)/2/2-((14))+j+shift," ");
            attron(COLOR_PAIR(theme));}
    return size+6;
}


void welcome(){
    int shift=10;
    char Shapes[7][2][8]={
        {"        ","AAAAAAAA"},
        {"AAAAAA","    AA"},
        {"AAAAAA","AA    "},
        {"AAAA","AAAA"},
        {"  AAAA","AAAA  "},
        {"AAAAAA","  AA  "},
        {"AAAA  ","  AAAA"}
    };
    for (int i=0;i<7;i++)
        shift+=draw_Shape(2,8,Shapes[i],shift);
    shift+=62;
    for (int i=0;i<8;i++)
        shift+=draw_Shape(2,8,Shapes[7-i],shift);
    char tetris[7][66]={
        "AAAAAAAAAA  AAAAAAAAAA  AAAAAAAAAA  AAAAAAAA    AAAAAA    AAAAAA  ",
        "    AA      AA              AA      AA      AA    AA    AA      AA",
        "    AA      AA              AA      AA      AA    AA    AA        ",
        "    AA      AAAAAAAA        AA      AAAAAAAA      AA      AAAAAA  ",
        "    AA      AA              AA      AA  AA        AA            AA",
        "    AA      AA              AA      AA    AA      AA    AA      AA",
        "    AA      AAAAAAAAAA      AA      AA      AA  AAAAAA    AAAAAA  "
    };
    for (int i=0;i<7;i++)
        for (int j=0;j<66;j++){
            if (tetris[i][j]=='A'){
                attron(COLOR_PAIR(theme+1));
                mvprintw(i+5,j+COL/2-33," ");
                attron(COLOR_PAIR(theme));
            }
            else{

                mvprintw(i+1,j+1," ");
            }
            }
    attron(A_BLINK);
    mvaddstr(ROW/2+1, COL/2-(28/2),"Press any key to continue...");
    attron(A_NORMAL);
    refresh();
    getch();
    move(0,0);
}

void main_menu(){
    //
    bkgd(COLOR_PAIR(theme));
    WINDOW* menuwin= newwin(5,40,ROW/2-2,COL/2-20);
    char choises[3][9] = {"Play", "Settings", "Exit"};
    int choise=0;
    wbkgd(menuwin, COLOR_PAIR(theme));
    box(menuwin,0,0);
    mvwprintw(menuwin, 0, 40/2-4, "| HOME |");
    while (1){
        refresh();
        for (int i=0;i<3;i++){
            if (i==choise){
                wattron(menuwin, COLOR_PAIR(theme+1));
                mvwprintw(menuwin, i+1,1,choises[i]);
                wattron(menuwin, COLOR_PAIR(theme));
            }
            else 
                mvwprintw(menuwin, i+1,1,choises[i]);
            refresh();
        }
        switch (wgetch(menuwin))
        {
        case 'w':
            choise--;
            if (choise==-1)
                choise=0;
            break;

        case 's':
            choise++;;
            if (choise==3)
                choise=2;;
            break;
        case '\n':
            wmove(menuwin,0,0);
            wclrtobot(menuwin);
            wrefresh(menuwin);
            delwin(menuwin);
            switch (choise)
            {
            case 0:
                return play_menu();
                break;
            case 1:
                return settings_menu();
                break;
            case 2:
                return;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    wrefresh(menuwin);
    getch();
}

void play_menu(){
    bkgd(COLOR_PAIR(theme));
    WINDOW* menuwin= newwin(5,40,ROW/2-2,COL/2-20);
    char choises[3][53] = {"'A'/'D to move 'Enter' to confirm" , " \0        ", "12345678"};
    int level=1;
    int choise=1;
    wbkgd(menuwin, COLOR_PAIR(theme));
    box(menuwin,0,0);
    mvwprintw(menuwin, 0, 40/2-7, "| DIFFICULTY |");
    mvwprintw(menuwin, 0+1,3,choises[0]);
    //└──────────────────────────────────────┘
    while(1){
        
        refresh();
        for (int i=1;i<3;i++){
            if (i==choise){
                move(i+1,19-3);
                wattron(menuwin, COLOR_PAIR(theme+1));
                mvwprintw(menuwin, i+1,19-3,choises[i]);
                wattron(menuwin, COLOR_PAIR(theme));
                mvwprintw(menuwin, i+1,19-3+level,"            ");
            }
            else 
                mvwprintw(menuwin, i+1,19-3,choises[i]);
            refresh();
        }
        switch (wgetch(menuwin))
        {
        case 'a':
            choises[1][level]=' ';
            level--;
            if (level==0)
                level=1;
            choises[1][level]='\0';
            break;

        case 'd':
            choises[1][level]=' ';
            level++;;
            if (level==9)
                level=8;
            choises[1][level]='\0';
            break;
        case '\n':
            wmove(menuwin,0,0);
            wclrtobot(menuwin);
            wrefresh(menuwin);
            delwin(menuwin);
            return game(level);
            break;
        default:
            break;
        }
    }
}

void settings_menu(){
    bkgd(COLOR_PAIR(theme));
    WINDOW* menuwin= newwin(5,40,ROW/2-2,COL/2-20);
    char choises[3][18] = {"Theme", "Initial screen", "Turn back"};
    int choise=0;
    wbkgd(menuwin, COLOR_PAIR(theme));
    box(menuwin,0,0);
    mvwprintw(menuwin, 0, 40/2-6, "| SETTINGS |");
    while(1){
        refresh();
        for (int i=0;i<3;i++){
            if (i==choise){
                wattron(menuwin, COLOR_PAIR(theme+1));
                mvwprintw(menuwin, i+1,1,choises[i]);
                wattron(menuwin, COLOR_PAIR(theme));
            }
            else 
                mvwprintw(menuwin, i+1,1,choises[i]);
            refresh();
        }
        switch (wgetch(menuwin))
        {
        case 'w':
            choise--;
            if (choise==-1)
                choise=0;
            break;

        case 's':
            choise++;;
            if (choise==3)
                choise=2;;
            break;
        case '\n':
            wmove(menuwin,0,0);
            wclrtobot(menuwin);
            wrefresh(menuwin);
            delwin(menuwin);
            switch (choise)
            {
            case 0:
                return theme_menu();
                break;
            case 1:
                return initial_menu();
                break;
            case 2:
                return main_menu();
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

void theme_menu(){
    WINDOW* menuwin= newwin(5,40,ROW/2-2,COL/2-20);
    char choises[3][18] = {"Violet", "Dark", "Turn back"};
    int choise=0;
    wbkgd(menuwin, COLOR_PAIR(theme));
    box(menuwin,0,0);
    mvwprintw(menuwin, 0, 40/2-5, "| THEME |");
    while(1){
        refresh();
        for (int i=0;i<3;i++){
            if (i==choise){
                wattron(menuwin, COLOR_PAIR(theme+1));
                mvwprintw(menuwin, i+1,1,choises[i]);
                wattron(menuwin, COLOR_PAIR(theme));
            }
            else 
                mvwprintw(menuwin, i+1,1,choises[i]);
            refresh();
        }
        switch (wgetch(menuwin))
        {
        case 'w':
            choise--;
            if (choise==-1)
                choise=0;
            break;

        case 's':
            choise++;;
            if (choise==3)
                choise=2;;
            break;
        case '\n':
            wmove(menuwin,0,0);
            wclrtobot(menuwin);
            wrefresh(menuwin);
            delwin(menuwin);
            switch (choise)
            {
            case 0:
                theme=1;
                FILE *settings1=fopen("settings.txt","w");
                fputc('1',settings1);
                fputc(' ',settings1);
                fputc(welc+'0',settings1);
                fclose(settings1);
                return settings_menu();
                break;
            case 1:
                theme=3;
                FILE *settings3=fopen("settings.txt","w");
                fputc('3',settings3);
                fputc(' ',settings3);
                fputc((welc+'0'),settings3);
                fclose(settings3);
                return settings_menu();
                break;
            case 2:
                return settings_menu();
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

void initial_menu(){
    bkgd(COLOR_PAIR(theme));
    WINDOW* menuwin= newwin(5,40,ROW/2-2,COL/2-20);
    char choises[3][18] = {"Default", "None", "Turn back"};
    int choise=0;
    wbkgd(menuwin, COLOR_PAIR(theme));
    box(menuwin,0,0);
    mvwprintw(menuwin, 0, 40/2-9, "| INITIAL SCREEN |");
    while(1){
        refresh();
        for (int i=0;i<3;i++){
            if (i==choise){
                wattron(menuwin, COLOR_PAIR(theme+1));
                mvwprintw(menuwin, i+1,1,choises[i]);
                wattron(menuwin, COLOR_PAIR(theme));
            }
            else 
                mvwprintw(menuwin, i+1,1,choises[i]);
            refresh();
        }
        switch (wgetch(menuwin))
        {
        case 'w':
            choise--;
            if (choise==-1)
                choise=0;
            break;

        case 's':
            choise++;;
            if (choise==3)
                choise=2;;
            break;
        case '\n':
            wmove(menuwin,0,0);
            wclrtobot(menuwin);
            wrefresh(menuwin);
            delwin(menuwin);
            switch (choise)
            {
            case 0:
                welc=1;
                FILE *settings1=fopen("settings.txt","w");
                fputc(theme+'0',settings1);
                fputc(' ',settings1);
                fputc('1',settings1);
                fclose(settings1);
                return settings_menu();
                break;
            case 1:
                welc=0;
                FILE *settings0=fopen("settings.txt","w");
                fputc(theme+'0',settings0);
                fputc(' ',settings0);
                fputc('0',settings0);
                fclose(settings0);
                return settings_menu();
                break;
            case 2:
                return settings_menu();
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

void end_menu(){
    bkgd(COLOR_PAIR(theme));
    WINDOW* menuwin= newwin(0,0,0,0);
    wbkgd(menuwin,COLOR_PAIR(theme));
    wattron(menuwin, COLOR_PAIR(theme));
    char gameover[7][104]={
        "  AAAAAA        AA      AA          AA  AAAAAAAAAA          AAAAAA    AA      AA  AAAAAAAAAA  AAAAAAAA  ",
        "AA            AA  AA    AAAA      AAAA  AA                AA      AA  AA      AA  AA          AA      AA",
        "AA            AA  AA    AA  AA  AA  AA  AA                AA      AA  AA      AA  AA          AA      AA",
        "AA  AAAAAA  AA      AA  AA    AA    AA  AAAAAAAA          AA      AA    AA  AA    AAAAAAAA    AAAAAAAA  ",
        "AA      AA  AAAAAAAAAA  AA          AA  AA                AA      AA    AA  AA    AA          AA  AA    ",
        "AA      AA  AA      AA  AA          AA  AA                AA      AA      AA      AA          AA    AA  ",
        "  AAAAAAAA  AA      AA  AA          AA  AAAAAAAAAA          AAAAAA        AA      AAAAAAAAAA  AA      AA"
    };
    for (int j=0;j<7;j++){
        wmove(menuwin,ROW/2-10+j,COL/2-(52));
        for (int i=0;i<104;i++){
            if (gameover[j][i]=='A'){
                wattron(menuwin, COLOR_PAIR(theme+1));
                waddch(menuwin,' ');
                wattron(menuwin, COLOR_PAIR(theme));
                }
            else 
                waddch(menuwin,' ');
        }
    }

    wattron(menuwin,A_BLINK);
    mvwprintw(menuwin, ROW/2, COL/2-(21),"Press 'Enter' to continue or any key to exit");
    wattron(menuwin,A_NORMAL);
    wrefresh(menuwin);
        switch (wgetch(menuwin))
        {
        case '\n':
            wmove(menuwin,0,0);
            wclrtobot(menuwin);
            wrefresh(menuwin);
            delwin(menuwin);
            return main_menu();
            break;
        default:
            break;
        }
    wrefresh(menuwin);
}