#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

//constant variables
int X = 3;
int opponent = 1;// user plays as o
int player = -1; //computer plays as x                                            |          |
//                                                                                |          |
int places = 9;//                                                        place[0] | place[1] | place[2]              x = -1
int number[] = {1, 0, -1};//                                             _________|__________|_________              - = 0
//                                                                                |          |                       o = 1
int place[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};//                             place[3] | place[4] | palce[5]      --> {o , - , x} = {1, 0, -1}
                                          //                             _________|__________|________
int numbers = sizeof(number) / sizeof(number[0]);//                               |          |
int repeat = 3;//                                                        place[6] | place[7] | place[8]
//                                                                                |          |
//                                                                                |          |
int game[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
// big tables live in global memory: as locals in main they overflowed the 1MB stack on Windows
#define PLACES 9
#define MAXC 19683 // 3^9
static int combination[PLACES][MAXC][PLACES];
// two tables: [0] = games where you (o) start, [1] = games where the computer (x) starts
// (with one table that assumed o always starts, x's opening moves got positive scores)
static float values[2][PLACES][MAXC];
static int futures[2][PLACES][MAXC];
int first = 1; // who made the first move of the game: opponent (o) or player (x)
// text shown to the right of the board: the score of the move that was just made
char last_move[64] = "";

//functions
int combinations(int i, int j);
int division_int(int x, int y);
int change(int p, int first_number, int last_number);
int accurate(int combination[]);
int win(int str[], int n);
int who_plays(int combo[]);
int row(int combo[]);
int next(int combo[], int p, int r, int turn, int roundd);
int play(int g[]);
int choose(float value[]);
int print(void);
float move_score(int g[], int combo_row[], float value[][MAXC]);
int set_last_move(int who, int spot, float s);


int main(void){
    printf("loading game...\n");
    int c = 1;
    for(int pw = 0; pw < places; pw++) c *= numbers; // integer 3^9 (pow() can round down to 19682)
    int combo_row[places];
    for(int zer = 0; zer < places; zer++) combo_row[zer] = 0;


    

    for(int r = 0; r < places; r++){//                                                                     (1) generating all possible outcomes of the game
        for(int k = 0; k < c; k++){
            int Combo[places];
            for(int i = 0; i < places; i++) Combo[i] = 0;
            for(int j = 0; j < places; j++){
                Combo[j] = combinations(k, j);
            }
            if(accurate(Combo) <= 1 && row(Combo) == r){
                for(int e = 0; e < places; e++) combination[r][combo_row[r]][e] = Combo[e];
                values[0][r][combo_row[r]] = values[1][r][combo_row[r]] = accurate(combination[r][combo_row[r]]) * 1000; //starting value of 1000(o wins), -1000(x wins) or 0(no wins)  *1000 for more accuracy
                combo_row[r]++;
            }
        }
    }//to see all combinations and their values, assign now = 1
    int now = 0;           //                                                                x| |o
    //                     // the future feature anables it to recognise a dead end  f.e     _|x|_  (dead end)
    int countf = 0;        //                                                                o| |o
    int yes = 0;
    for(int t = 0; t < 2; t++){ // t = 0: o starts, t = 1: x starts
    first = (t == 0) ? opponent : player;
    int startround = t; // passed to next() as roundd so that on equal counts the starter moves
    float (*value)[MAXC] = values[t];
    int (*future)[MAXC] = futures[t];
                            //                                                                             (2) giving a value to each possible outcome, based on what it can lead to
    for(int p = places - 2; p >= 0; p--){ // starts from the second last row and goes backwards
        for(int l = 0; l < combo_row[p]; l++){
            if(value[p][l] == 0){
            yes = 0;
            countf = 0;
                for(int w = 0; w < places - p - 1; w++){
                    int combo[places];
                    for(int o = 0; o < places; o++){
                        combo[o] = next(combination[p][l], o, w, 0, startround);
                    }
                    for(int y = 0; y < combo_row[p + 1]; y++){
                        int s = 0, zero = 0;
                        for(int q = 0; q < places; q++){
                            if(combo[q] == combination[p + 1][y][q]) s++;
                            if(combo[q] == 0) zero++;
                        }
                        if(s == places){
                            if(value[p + 1][y] == who_plays(combination[p][l]) * 1000){
                                future[p][l] = 1;
                            }

                            if(future[p + 1][y] == 1) {
                                countf++;
                            }
                            if(future[p + 1][y] == 2){
                                yes = 1;
                            }
                            value[p][l] += value[p + 1][y];

                        }
                        if(zero == places) value[p][l] = 0;
                    }
                }
                value[p][l] /= places - p;
                if(countf == places - p - 1){

                    future[p][l] = 2;
                }
                if(yes == 1) {
                    value[p][l] = who_plays(combination[p][l]) * 1000;
                }
            }
            if(now == 1){
                for(int ii = 0; ii < places; ii++) printf("%i", combination[p][l][ii]);
            }
            if(now == 1) printf("        %f \n", value[p][l]);
        }
    }
    } // end of the two tables

    int score[2] = {0, 0};
    int roundd = 0;//                                                    (3) while playing, it chooses the value closer to -1000 (computer is x) or 1000 (computer is o)
    while(1){
        for(int f = 0; f < places; f++) game[f] = 0;
        last_move[0] = '\0'; // new game: no move to show yet
        int turn = 0;
        int pl[places];
        float (*value)[MAXC] = values[division_int(roundd, 2)]; // even round: you start, odd round: computer starts
        while(accurate(game) == 0 && row(game) + 1 != places){
            now = 0;
            print();
            if(division_int(turn, 2) == division_int(roundd, 2)){
                int x;
                do{
                    printf(" (to exit, enter 0) \nenter a number: ");
                    scanf("%d", &x);
                    if(x == 0) return 0;
                }
                while(x < 1 || x > 9 || game[x - 1] != 0);
                game[x - 1] = opponent;
                set_last_move(opponent, x - 1, move_score(game, combo_row, value));
            }
            else{

                float val[places - (row(game) + 1)];
                for(int re = 0; re < places - (row(game) + 1); re++){
                    for(int m = 0; m < places; m++){
                        pl[m] = next(game, m, re, turn, roundd);
                    }
                    for(int v = 0; v < combo_row[row(game) + 1]; v++){
                        int S = 0, Zero = 0;
                        for(int qi = 0; qi < places; qi++){
                            if(pl[qi] == combination[row(game) + 1][v][qi]) S++;
                            if(pl[qi] == 0) Zero++;
                        }
                        if(S == places){
                            val[re] = value[row(game) + 1][v];
                        }
                        if(Zero == places) val[re] = 0;
                    }

                }
                int before[places];
                for(int b = 0; b < places; b++) before[b] = game[b];
                choose(val);
                for(int b = 0; b < places; b++){
                    if(before[b] != game[b]) set_last_move(player, b, move_score(game, combo_row, value));
                }
            }
            turn++;
        }

        print();
        if(accurate(game) == opponent){
            score[0]++;
            printf("you won!\n");
        }
        else if(accurate(game) == player){
            score[1]++;
            printf("i won!\n");
        }
        printf("score: you %d - computer %d\n", score[0], score[1]);
        fflush(stdout);

        roundd++;
        print();
    }
}
int combinations(int i, int j){
    int combo;
    int f[places];
    int k = 0;
    for(int o = places - 1; o >= 0; o--){
        f[o] = pow(numbers, k);
        k++;
    }
    if(division_int(i, f[j]) == 0 && i != 0){
        place[j] = change(place[j], number[0], number[numbers - 1]);
    }
    combo = place[j];
    return combo;
}

int division_int(int x, int y){
    int s = x / y;
    if((float)x / (float)y == (float)s){
        return 0;
    }
    else {return 1;}
}

int change(int p, int first_number, int last_number){
    if(p == last_number){
        p = first_number;
    }
    else{
        p--;
    }
    return p;
}


int accurate(int combination[]){
    int nx = 0;
    int no = 0;

    for(int i = 0; i < places; i++){
        if(combination[i] == -1) nx++;
        else if(combination[i] == 1) no++;
    }

    if(abs(nx - no) == 1 || abs(nx - no) == 0){

        int x[nx];
        int o[no];
        int k = 0; int l = 0;
        for(int j = 0; j < places; j++){
            if(combination[j] == -1){
                x[k] = j;
                k++;
            }
            else if(combination[j] == 1){
                o[l] = j;
                l++;
            }
        }
        if(nx == 0 && no == 0) return 0;
        int wx = win(x, nx);
        int wo = win(o, no);



        if(wx == 1 && wo == 0) return -1;
        else if(wo == 1 && wx == 0) return 1;
        else if(wx == 0 && wo == 0) return 0;
        if(wx > 1 && nx < repeat * wx) return -1;
        if(wo > 1 && no < repeat * wo) return 1;
        else return 2;
    }
    else return 2;
    return 2;
}
//
int win(int x[], int nx){//                      f.e.         positions of x = {3, 4, 5, 8}               _|_|_
    int wins = 0;//                   function generates:     3 4 5,  3 4 8,  3 5 8,  4 5 8               x|x|x
//                                                                5 - 4 == 4 - 3     ->   x wins          o|o|x
        if(nx >= repeat){
            int combo[] = {x[0], x[1], x[2]};
            int r = (nx - repeat) * nx;
            if(nx == repeat) r++;

            for(int p = 0; p < r; p++){
                int a = combo[repeat - 1] - combo[repeat - 2];
                int b = combo[repeat - 2] - combo[repeat - 3];
                if(a == b){
                    if(combo[0] / X != combo[1] / X && combo[1] / X != combo[2] / X && combo[0] / X != combo[2] / 4) wins++;
                    else if(combo[0] / X == combo[1] / X && combo[1] / X == combo[2] / X && combo[0] / X == combo[2] / 4) wins++;
                }

                int w = repeat - 1;
                for(int u = 0; u < repeat; u++){
                    int m = 0;
                    if(combo[u] != x[nx - (repeat - u)]){
                        for(int n = 1; n < repeat - u; n++){
                            if(combo[u + n] == x[nx - (repeat - (u + n))]) m++;
                        }
                        if(m == w - u){
                            int z = 0;
                            for(int t = 0; t < nx; t++) if(combo[u] == x[t]) z = t;
                            int f = 1;
                            for(int q = u; q < repeat; q++){
                                combo[q] = x[z + f];
                                f++;
                            }


                        u = repeat;
                        }
                    }
                }
            }
        }
        else return 0;
    return wins;
}
int who_plays(int combo[]){
    int nx = 0;
    int no = 0;

    for(int i = 0; i < places; i++){
        if(combo[i] == player) nx++;
        else if(combo[i] == opponent) no++;
    }

    if(nx < no) return player;
    else if(no < nx) return opponent;
    else return first; // equal counts: whoever started the game moves
}

int row(int combo[]){
    int n = 0;
    for(int i = 0; i < places; i++){
        if(combo[i] == 1 || combo[i] == -1) n++;
    }
    return n - 1;
}

int next(int combo[], int p, int r, int turn, int roundd){

    int nplayer = 0;
    int nopponent = 0;

    for(int i = 0; i < places; i++){
        if(combo[i] == player) nplayer++;
        else if(combo[i] == opponent) nopponent++;
    }

    int spot = 0;
    if(nopponent > nplayer){
        if(combo[p] == 0){
            for(int i = 0; i < p; i++){
                if(combo[i] == 0) spot++;
            }
            if(spot == r) return player;
            else return 0;
        }
        else return combo[p];
    }
    else if(nopponent < nplayer){
        if(combo[p] == 0){
            for(int s = 0; s < p; s++){
                if(combo[s] == 0) spot++;
            }
            if(spot == r) return opponent;
            else return 0;
        }
        else return combo[p];
    }
    else if(nopponent == nplayer){
        if(combo[p] == 0){
            for(int s = 0; s < p; s++){
                if(combo[s] == 0) spot++;
            }
            if(spot == r){
                if(division_int(turn, 2) == division_int(roundd, 2)) return opponent;
                else return player;
            }
            else return 0;
        }
        else return combo[p];
    }
    return 0;
}

int choose(float value[]){
    int pick = 0;
    for(int i = 1; i < places - row(game) - 1; i++){
        float x = fabsf((float)player * 1000 - value[pick]);
        float y = fabsf((float)player * 1000 - value[i]);
        if(x > y){
            pick = i--;
        }
    }
    int zero = 0;
    for(int j = 0; j < places; j++){
        if(game[j] == 0){
            if(zero == pick) game[j] = player;
            zero++;
        }
    }
    return 0;
}
int print(void){
    int xox = 0;
    for(int i = 0; i < places; i++){
        for(int j = 0; j < X; j++){
            for(int l = 0; l < 6; l++){
                if(l == 5 && j != X - 1) printf("|");
                else if(l == 5 && j == X - 1){
                    if(i == 4 && last_move[0] != '\0') printf("        %s", last_move); // middle row of the board
                    printf("\n");
                }
                else if(l == 2 && division_int(i - 1, 3) == 0){
                    if(game[xox] == 1) printf("O");
                    else if(game[xox] == -1) printf("X");
                    else printf(" ");
                    xox++;
                }
                else if(i != places - 1 && division_int(i + 1, 3) == 0 && i != 0) printf("_");
                else printf(" ");
            }
        }

    }
    printf("\n");
    return 0;
}

// finds the board g in the precomputed tables and returns its value (the score of the move that led to it)
// the closer to 1000, the better for o (you); the closer to -1000, the better for x (computer)
float move_score(int g[], int combo_row[], float value[][MAXC]){
    int r = row(g);
    if(r < 0) return 0;
    for(int v = 0; v < combo_row[r]; v++){
        int same = 0;
        for(int q = 0; q < places; q++) if(g[q] == combination[r][v][q]) same++;
        if(same == places) return value[r][v];
    }
    return 0;
}

int set_last_move(int who, int spot, float s){
    if(who == opponent) sprintf(last_move, "you (O) played %d   score: %.1f", spot + 1, s);
    else sprintf(last_move, "computer (X) played %d   score: %.1f", spot + 1, s);
    return 0;
}
