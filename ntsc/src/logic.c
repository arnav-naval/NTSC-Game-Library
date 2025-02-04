#include "common.h"
#include "clock.h"
#include "video.h"
#include "draw.h"
#include "sprite.h"
#include "tilemap.h"
#include "render.h"
#include "flex.h"
#include "graphics.h"

//for testing purposes
// int all_cards[52] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 52};
// int all_cards[52] = {37, 13, 24, 25, 2, 12, 9, 20, 15, 18, 46, 14, 5, 1, 23, 47, 30, 50, 43, 49, 51, 27, 3, 19, 38, 28, 45, 4, 11, 17, 39, 36, 41, 29, 10, 7, 32, 40, 34, 33, 16, 48, 0, 21, 26, 35, 44, 8, 31, 22, 42, 6};
// int card_increment = 0;
int card;

int win; // true if win
int lose;  //true if lose
int double_down; //true if user chose to double down
int tie; //true if there is a tie
uint32_t joystickX; //hold joystick position on X axis
uint32_t joystickY; //hold joystick position on Y axis

int players_cards[11] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int dealer_cards[11] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int player_card_count;

int used_cards[52];
int used_cards_count;

int happy_sound_trigger = 0;
int sad_sound_trigger = 0;


int count = 0;
int prev_random_number = 1;
int getRandomNumber(){
    count++;
    int seed = count;
    int random_number = seed * 1664525 + 1013904223 + joystickX * prev_random_number + joystickY;
    random_number = random_number % 52;
    prev_random_number = random_number;
    return random_number;
}

void reset_cards(){
    used_cards_count = 0;
    for(int i = 0; i < 52; i++){
        used_cards[i] = -1;
    }
}

//draw card function to be used with random_number generator
int draw_card(){
    int card;
    if(used_cards_count >= 52){
        reset_cards();
    }
    while(1){
        card = getRandomNumber();
        int is_used = 0;
        for(int i = 0; i < used_cards_count; i++){
            if(used_cards[i] == card){
                is_used = 1;
                count++;
                draw_card(); //i hate recursion
            }
        }
        if(!is_used){
            used_cards[used_cards_count] = card;
            used_cards_count++;
            return card;
        }
    }
}
/*
//draw_card function to be used with static card array, use only in testing
int draw_card(){
    if(card_increment < 51){
        card = (all_cards[card_increment] + ADC1->DR * 134343252) % 52;
        card_increment++;
        return card;
    }
    else{
        card_increment = 0;
        return draw_card();
    }
}
*/


int32_t readpin(int32_t pin_num) { //read the value of an input pin
    // count += 1;
    return (GPIOB->IDR >> pin_num ) & 1;
}

//calculate the points for every card but aces
int points;
int calculate_card_points(int card){
    int card_number = card % 13;
    points = 0;
    if(card_number == 0){
        //an ace can either be 1 or 11
        //extra function for ace condition?
        points = 0;
    }
    else if(card_number <= 9){
        //since 0 is ace, 1 is 2, 2 is 3, etc.
        points = card_number + 1;
    }
    else{
        points = 10;
    }
    return(points);
}

//only job is to return true if the card is an ace
int ace_check(int card){ //where card is the 0-51 card number
    if((card % 13) == 0){
        return 1;
    }
    else{
        return 0;
    }
}

//handles adding the value of all aces to the total
int choose_ace_value(int number_aces, int total_no_aces){
    //if there is only one ace
    if(number_aces == 1){
        if((total_no_aces + 11) <= 21){
            return (total_no_aces + 11);
        }
        else{
            return(total_no_aces + 1);
        }
    }
        
    //if there is two or more aces
    else{
        //two aces cant be 11, so check if one is 11 and the others are all 1
        if((total_no_aces + 11 + (number_aces - 1)) <= 21){
            return(total_no_aces + 11 + (number_aces - 1));
        }
        //if not, all will have to be 1
        else{
            return(total_no_aces + number_aces);
        }
    }
}

int stand_or_hit();

#define CARD1X 50
#define CARD1Y 75
#define CARD2X 85
#define CARD2Y 75
#define CARD3X 15
#define CARD3Y 80
#define CARD4X 120
#define CARD4Y 80

struct Sprite p1;
struct Sprite p2;
struct Sprite p3;
struct Sprite p4;
struct Sprite d1;
struct Sprite d2;
struct Sprite d3;

struct SpriteString d_total;
struct SpriteString d1_total;
struct SpriteString p_total;
struct SpriteString p1_total;

struct SpriteString pbust;
struct SpriteString dbust;

uint8_t p1_card_id;
uint8_t p2_card_id;
uint8_t p3_card_id;
uint8_t p4_card_id;

void shuffle_cards(){
    int total_shifts = player_card_count - 5;
    int shifts = 0;
    int current_p1 = 0;
    int current_p2 = 1;
    int current_p3 = 2;
    int current_p4 = 3;
    while(1){
        if((joystickX <= 1000) && (joystickY >= 1700 && joystickY <= 2300)){ //joystick position left
        //  SHIFT CARDS LEFT
            if(shifts < total_shifts){
                shifts++;
                current_p1++;
                current_p2++;
                current_p3++;
                current_p4++;
                change_card(&p3, players_cards[current_p1]);
                change_card(&p1, players_cards[current_p2]);
                change_card(&p2, players_cards[current_p3]);
                change_card(&p4, players_cards[current_p4]);
            }
            
        }
        if((joystickX >= 3000) && (joystickY >= 1700 && joystickY <= 2300)){ //joystick position right
        //SHIFT CARDS RIGHT
            if(shifts > 0){
                shifts--;
                current_p1--;
                current_p2--;
                current_p3--;
                current_p4--;
                change_card(&p3, players_cards[current_p1]);
                change_card(&p1, players_cards[current_p2]);
                change_card(&p2, players_cards[current_p3]);
                change_card(&p4, players_cards[current_p4]);       
            }
        }
        if((joystickX >= 1700 && joystickX <= 2300) && (joystickY <= 1000)){ //joystick position up
            break;
        }
        render_pass();
    }
}


void play_blackjack(){
    int player_ace = 0; //equal to the number of aces in the players hand
    int dealer_ace = 0; //the same thing for the dealer
    int bust = 0; //becomes true if the hand is a bust
    player_card_count = 2;
    
    //dealer face down card
    // struct Sprite d1;
    d1.x = 55;
    d1.y = 10;
    // uint8_t d1_card_id = 52;
    init_card(&d1, 52, 55, 10);

    //dealer face up card
    // struct Sprite d2;
    d2.x = 80;
    d2.y = 10;
    uint8_t d2_card_id = draw_card();
    dealer_cards[0] = d2_card_id;
    init_card(&d2, d2_card_id, 80, 10);

    //dealers third card
    d3.x = 25;
    d3.y = 10;
    uint8_t d3_card_id = draw_card();
    dealer_cards[2] = d3_card_id;
    init_card(&d3, d3_card_id, 25, 10);

    //players first card
    // struct Sprite p1;
    p1.x = CARD1X;
    p1.y = CARD1Y;
    p1_card_id = draw_card();
    players_cards[1] = p1_card_id;
    init_card(&p1, p1_card_id, CARD1X, CARD1Y);

    //players second card
    // struct Sprite p2;
    p2.x = CARD2X;
    p2.y = CARD2Y;
    p2_card_id = draw_card();
    players_cards[2] = p2_card_id;
    init_card(&p2, p2_card_id, CARD2X, CARD2Y);

    //position for 3rd card
    // struct Sprite p3;
    p3.x = CARD3X;
    p3.y = CARD3Y;
    p3_card_id = draw_card();
    players_cards[0] = p3_card_id;
    init_card(&p3, p3_card_id, CARD3X, CARD3Y);

    //position for 4th card
    // struct Sprite p4;
    p4.x = CARD4X;
    p4.y = CARD4Y;
    p4_card_id = draw_card();
    players_cards[3] = p4_card_id;
    init_card(&p4, p4_card_id, CARD4X, CARD4Y);

    sprite_draw(&p1);
    sprite_draw(&p2);
    sprite_draw(&d1);
    sprite_draw(&d2);
    render_pass();

    uint8_t d1_card_id = draw_card(); //ITS NOT SHOWN YET BUT ITS CALCULATED


    player_ace += ace_check(p1_card_id); //check if its an ace, add to ace counter
    player_ace += ace_check(p2_card_id);
    dealer_ace += ace_check(d1_card_id);
    dealer_ace += ace_check(d2_card_id);

    //calculating initial totals
    int player_total = calculate_card_points(p1_card_id) + calculate_card_points(p2_card_id);
    int dealer_total = calculate_card_points(d1_card_id) + calculate_card_points(d2_card_id);

    //PLAYER BUSTS MESSAGE
    char bust_str[] = "BUST";
    // struct SpriteString pbust;
    struct Sprite bust_string[sizeof(bust_str) / sizeof(bust_str[0]) - 1];
    pbust.arr = bust_string;
    pbust.len = sizeof(bust_str) / sizeof(bust_str[0]) - 1;
    init_string(&pbust, bust_str, sizeof(bust_str) - 1, 70, 45);

    //DEALER BUSTS MESSAGE
    char dbust_str[] = "DEALER BUST";
    // struct SpriteString dbust;
    struct Sprite dbust_string[sizeof(dbust_str) / sizeof(dbust_str[0]) - 1];
    dbust.arr = dbust_string;
    dbust.len = sizeof(dbust_str) / sizeof(dbust_str[0]) - 1;
    init_string(&dbust, dbust_str, sizeof(dbust_str) - 1, 55, 55);

//hit or stand functions
    while(1){
        // count++;
        if((player_ace == 1) && (player_total == 10)){
            break;
            // return 0;
        }
        int choice = stand_or_hit();
        //the variable choice is the number for the card you draw
        if(choice == -1){
            break;
            //game is over see if you won
        }

        if(player_card_count == 3){
            choice = p3_card_id;
            sprite_draw(&p3);
        }
        else if(player_card_count == 4){
            choice = p4_card_id;
            sprite_draw(&p4);
        }
        else{
            //puts a new card in the array, but you cant see it yet
            choice = draw_card();
            players_cards[player_card_count - 1] = choice;
        }

        player_ace += ace_check(choice);
        player_total += calculate_card_points(choice);
        //assumes all the aces are 1 to check for a bust
        if(player_total + player_ace > 21){
            //DISPLAY BUST MESSAGE
            draw_string(&pbust);
            render_pass();
            bust = 1;
            break;
        }
        if(double_down == 1){
            break;
        }
    }
    //add any aces onto the final totals
    if(player_ace != 0){
        player_total = choose_ace_value(player_ace, player_total);
    }
    
    int dealer_bust = 0; //true if the dealer busts

    //the dealer will automatically draw more cards while their total is below 16
    int dealer_draws = 0;
    while(!(((dealer_total + dealer_ace) > 16) || ((dealer_ace >= 1) && ((dealer_total + dealer_ace) > 6)))){
        if(dealer_draws != 0){
            break;
        }
        // count++;
        if((dealer_ace == 1) && (dealer_total == 10)){
            break;
        }
        //automatically draws card if the while condition is true
        // int choice = draw_card();
        dealer_ace += ace_check(d3_card_id);
        dealer_total += calculate_card_points(d3_card_id);
        nano_wait(200000000);
        sprite_draw(&d3);
        render_pass();
       //display the new card the dealer drew
        //assumes all the aces are 1 to check for a bust
        if(dealer_total + dealer_ace > 21){
            //DISPLAY DEALER BUSTS MESSAGE
            dealer_bust = 1;
            break;
        }
        dealer_draws++;
    }

    //add dealers aces to the final total
    if(dealer_ace != 0){
        dealer_total = choose_ace_value(dealer_ace, dealer_total);
    }

    if(bust == 0 && dealer_bust == 0){
        //DISPLAY DEALER'S FIRST CARD
        // change_card(&d1, d1_card_id);
        render_pass();

        //DISPLAY DEALER'S TOTAL
        char total_str[] = "DEALER TOTAL";
        // struct SpriteString d_total;
        struct Sprite sp_string4[sizeof(total_str) / sizeof(total_str[0]) - 1];
        d_total.arr = sp_string4;
        d_total.len = sizeof(total_str) / sizeof(total_str[0]) - 1;
        init_string(&d_total, total_str, sizeof(total_str) - 1, 45, 45);
        draw_string(&d_total);

        char total1_str[3];
        if(dealer_total >= 10){
            total1_str[0] = (char)((dealer_total / 10) + 48);
        }
        else{
            total1_str[0] = (char)32;
        }
        total1_str[1] = (char)((dealer_total % 10) + 48); 
        total1_str[2] = '\0';
        // struct SpriteString d1_total;
        struct Sprite sp_string5[sizeof(total1_str) / sizeof(total1_str[0]) - 1];
        d1_total.arr = sp_string5;
        d1_total.len = sizeof(total1_str) / sizeof(total1_str[0]) - 1;
        init_string(&d1_total, total1_str, sizeof(total1_str) - 1, 110, 45);
        draw_string(&d1_total);

        //DISPLAY PLAYER'S TOTAL

        char total2_str[] = "YOUR TOTAL";
        // struct SpriteString p_total;
        struct Sprite sp_string6[sizeof(total2_str) / sizeof(total2_str[0]) - 1];
        p_total.arr = sp_string6;
        p_total.len = sizeof(total2_str) / sizeof(total2_str[0]) - 1;
        init_string(&p_total, total2_str, sizeof(total2_str) - 1, 45, 55);
        draw_string(&p_total);

        char total3_str[3];
        if(player_total >= 10){
            total3_str[0] = (char)((player_total / 10) + 48);
        }
        else{
            total3_str[0] = (char)32;
        }
        total3_str[1] = (char)((player_total % 10) + 48); 
        total3_str[2] = '\0';
        // struct SpriteString p1_total;
        struct Sprite sp_string7[sizeof(total3_str) / sizeof(total3_str[0]) - 1];
        p1_total.arr = sp_string7;
        p1_total.len = sizeof(total3_str) / sizeof(total3_str[0]) - 1;
        init_string(&p1_total, total3_str, sizeof(total3_str) - 1, 110, 55);
        draw_string(&p1_total);

        render_pass();

        if((player_total > dealer_total) && (player_total <= 21)){
         //print "you win" message
            win = 1;
            lose = 0;
            tie = 0;
        }
        else if(player_total < dealer_total){
           //print "you lose" message
            lose = 1;
            win = 0;
            tie = 0;
        }
        else{
           //print "tie" message
            tie = 1;
            win = 0;
            lose = 0;
        }
    }
    else{
        if(dealer_bust == 1 && bust == 1){
            draw_string(&pbust);
            draw_string(&dbust);
            render_pass();
            tie = 1;
            win = 0;
            lose = 0;
        }
        else if(dealer_bust == 1 && bust == 0){
       //print "you win" message
            draw_string(&dbust);
            render_pass();
            win = 1;
            lose = 0;
            tie = 0;
        }
        else{
            lose = 1;
            win = 0;
            tie = 0;
        }
    }
    change_card(&d1, d1_card_id);
    render_pass();
    print_results();
    used_cards_count = 0;
}

void print_results(){
    char win_str[] = "YOU WIN";
    struct SpriteString winS;
    struct Sprite win_string[sizeof(win_str) / sizeof(win_str[0]) - 1];
    winS.arr = win_string;
    winS.len = sizeof(win_str) / sizeof(win_str[0]) - 1;
    init_string(&winS, win_str, sizeof(win_str) - 1, 60, 65);

    char lose_str[] = "YOU LOSE";
    struct SpriteString loseS;
    struct Sprite lose_string[sizeof(lose_str) / sizeof(lose_str[0]) - 1];
    loseS.arr = lose_string;
    loseS.len = sizeof(lose_str) / sizeof(lose_str[0]) - 1;
    init_string(&loseS, lose_str, sizeof(lose_str) - 1, 60, 65);

    char tie_str[] = "TIE";
    struct SpriteString tieS;
    struct Sprite tie_string[sizeof(tie_str) / sizeof(tie_str[0]) - 1];
    tieS.arr = tie_string;
    tieS.len = sizeof(tie_str) / sizeof(tie_str[0]) - 1;
    init_string(&tieS, tie_str, sizeof(tie_str) - 1, 70, 65);


    if(win == 1){
      draw_string(&winS);
      render_pass();
      happy_sound_trigger = 1;
    }
    if(lose == 1){
      draw_string(&loseS);
      render_pass();
      sad_sound_trigger = 1;
    }
    if(tie == 1){
      draw_string(&tieS);
      render_pass();
    }


    nano_wait(1000000000); //just let me have it
    sprite_erase(&d1);
    sprite_erase(&d2);
    sprite_erase(&d3);
    sprite_erase(&p1);
    sprite_erase(&p2);
    sprite_erase(&p3);
    sprite_erase(&p4);
    erase_string(&d_total);
    erase_string(&d1_total);
    erase_string(&p_total);
    erase_string(&p1_total);
    erase_string(&pbust);
    erase_string(&dbust);
    erase_string(&winS);
    erase_string(&loseS);
    erase_string(&tieS);
    render_pass();
    
}

int stand_or_hit(){
    // int choice =0; //choice = 1 for hit, 2 for double down, 0 for stand
    //default option is stand if player hits button without moving joystick
    int choice = 0;
    //INITIALIZE STRINGS
    char hit_str[] = " HIT ";
    char stand_str[] = " STAND ";
    char dd_str[] = " DOUBLE DOWN ";

    //PREPARE HIT MESSAGE
    struct SpriteString hit;
    struct Sprite sp_string[sizeof(hit_str) / sizeof(hit_str[0]) - 1];
    hit.arr = sp_string;
    hit.len = sizeof(hit_str) / sizeof(hit_str[0]) - 1;
    init_string(&hit, hit_str, sizeof(hit_str) - 1, 28, 60);

    //PREPARE STAND MESSAGE
    struct SpriteString stand;
    struct Sprite sp_string1[sizeof(stand_str) / sizeof(stand_str[0]) - 1];
    stand.arr = sp_string1;
    stand.len = sizeof(stand_str) / sizeof(stand_str[0]) - 1;
    init_string(&stand, stand_str, sizeof(stand_str) - 1, 101, 60);

    //PREPARE DOUBLE DOWN MESSAGE
    struct SpriteString dd;
    struct Sprite sp_string2[sizeof(dd_str) / sizeof(dd_str[0]) - 1];
    dd.arr = sp_string2;
    dd.len = sizeof(dd_str) / sizeof(dd_str[0]) - 1;
    init_string(&dd, dd_str, sizeof(dd_str) - 1, 48, 45);

    //DRAW THREE MESSAGES
    draw_string(&hit);
    draw_string(&stand);
    draw_string(&dd);
    render_pass();

    //PREPARE FIRST SELECTION DOT
    struct Sprite dot1;
    dot1.x = 98; //positioned on the left side of "stand" to start
    dot1.y = 61;
    uint8_t dot1_id = 59;
    init_dot(&dot1, dot1_id, 98, 61);

    //PREPARE SECOND SELECTION DOT
    struct Sprite dot2;
    dot2.x = 133;
    dot2.y = 61;
    uint8_t dot2_id = 59;
    init_dot(&dot2, dot2_id, 133, 61);

    //DRAW SELECT DOTS
    sprite_draw(&dot1);
    sprite_draw(&dot2);
    render_pass();
    
    //INCREMENT NUMBER OF CARDS IN PLAYERS HAND COUNTER
    player_card_count++;

    wait_frames(30);
    //IF BUTTON HAS NOT BEEN PRESSED
    while(!button_state){
        switch (joystick_state) {
        case JOYSTICK_UP:
            choice = 2; //double down
            //MOVE DOTS TO SURROUND DOUBLE DOWN
            sprite_move(&dot1, 45, 45);
            sprite_move(&dot2, 110, 45);
            break;
        case JOYSTICK_LEFT:
            choice = 1; //hit
            //MOVE DOTS TO SURROUND HIT
            sprite_move(&dot1, 25, 61);
            sprite_move(&dot2, 49, 61);
            break;
        case JOYSTICK_RIGHT:
            choice = 0;
            //MOVE DOTS TO SURROUND STAND
            sprite_move(&dot1, 98, 61);
            sprite_move(&dot2, 133, 61);
            break;
        case JOYSTICK_DOWN:
            //MOVE DOTS TO SURROUND BOTTOM CARDS IF THERE ARE MORE TO SEE THAN ON SCREEN ;
            if (choice == 2) {
                choice = 1; //hit
                //MOVE DOTS TO SURROUND HIT
                sprite_move(&dot1, 25, 61);
                sprite_move(&dot2, 49, 61);
            }
            else if (player_card_count >= 6) {
                sprite_move(&dot1, 5, 95);
                sprite_move(&dot2, 150, 95);
                shuffle_cards();
            }
            break;
        case JOYSTICK_IDLE:
            break;
        }
        render_pass();
        wait_frames(10);
    }
    //ERASE DOTS WHEN SELECTION IS MADE
    sprite_erase(&dot1);
    sprite_erase(&dot2);
    erase_string(&hit);
    erase_string(&stand);
    erase_string(&dd);

    render_pass();
    if((choice == 1) && (double_down == 0)){
        //IF CHOICE IS HIT, RETURN A CARD NUMBER
        return 1;
    }
    if(choice == 2){
        //IF CHOICE IS DOUBLE DOWN, SET DD INDICATOR TO 1 AND RETURN A CARD
        double_down = 1;
        return 1;
    }
    if(choice == 0){
        return -1;
    }
    else{
        //IF CHOICE IS STAND RETURN -1
        return -1;
    }
}
int total_chips; //the total number of chips the player is working with
void full_game(){
    //currently can handle max 99999 chips
    total_chips = 10;
    // int total_chips = 10; //the total number of chips the player is working with
    int round_chips; //the chips the player chooses to bet that round 
    int remaining_rounds = 5; //after 5 rounds the game will end, total chips will now be final score

    while(total_chips > 0 && remaining_rounds > 0){
        remaining_rounds -= 1;
        round_chips = 1;
        char total_prompt[] = "YOUR TOTAL CHIPS";
        struct SpriteString t_prompt;
        struct Sprite tp_string[sizeof(total_prompt) / sizeof(total_prompt[0]) - 1];
        t_prompt.arr = tp_string;
        t_prompt.len = sizeof(total_prompt) / sizeof(total_prompt[0]) - 1;
        init_string(&t_prompt, total_prompt, sizeof(total_prompt) - 1, 25, 40);

        //PRINT THE PLAYERS CURRENT TOTAL CHIPS

        char chips_str[6];
        if(total_chips >= 10000){
            chips_str[0] = (char)((total_chips / 10000) + 48);
        }
        else{
            chips_str[0] = (char)32;
        }   
        if(total_chips >= 1000){
            chips_str[1] = (char)(((total_chips % 10000) / 1000) + 48);
        }
        else{
            chips_str[1] = (char)32;
        }
        if(total_chips >= 100){
            chips_str[2] = (char)(((total_chips % 1000) / 100) + 48);
        }
        else{
            chips_str[2] = (char)32;
        }
        if(total_chips >= 10){
            chips_str[3] = (char)(((total_chips % 100) / 10) + 48);
        }
        else{
            chips_str[3] = (char)32;
        }
        chips_str[4] = (char)((total_chips % 10) + 48);
        chips_str[5] = '\0';
        struct SpriteString chips;
        struct Sprite chp_string[sizeof(chips_str) / sizeof(chips_str[0]) - 1];
        chips.arr = chp_string;
        chips.len = sizeof(chips_str) / sizeof(chips_str[0]) - 1;
        init_string(&chips, chips_str, sizeof(chips_str) - 1, 112, 40);


        //STRING WHICH CHANGES TO INDICATE BET
        char bchips_str[6];
        update_bet_string(round_chips, &bchips_str);

        struct SpriteString bchips;
        struct Sprite bchp_string[sizeof(bchips_str) / sizeof(bchips_str[0]) - 1];
        bchips.arr = bchp_string;
        bchips.len = sizeof(bchips_str) / sizeof(bchips_str[0]) - 1;
        init_string(&bchips, bchips_str, sizeof(bchips_str) - 1, 112, 65);

        //PROMPT FOR HOW MANY CHIPS TO BET
        char bet_prompt[] = "PLACE YOUR BET";
        struct SpriteString b_prompt;
        struct Sprite bp_string[sizeof(bet_prompt) / sizeof(bet_prompt[0]) - 1];
        b_prompt.arr = bp_string;
        b_prompt.len = sizeof(bet_prompt) / sizeof(bet_prompt[0]) - 1;
        init_string(&b_prompt, bet_prompt, sizeof(bet_prompt) - 1, 30, 65);

        //PREPARE FIRST SELECTION DOT
        struct Sprite dot1;
        dot1.x = 98; //positioned on the left side of "stand" to start
        dot1.y = 61;
        uint8_t dot1_id = 59;
        init_dot(&dot1, dot1_id, 122, 58);

        //PREPARE SECOND SELECTION DOT
        struct Sprite dot2;
        dot2.x = 133;
        dot2.y = 61;
        uint8_t dot2_id = 59;
        init_dot(&dot2, dot2_id, 122, 75);

        update_bet_string(round_chips, &bchips_str);
        draw_string(&t_prompt);
        draw_string(&chips);
        draw_string(&b_prompt);
        draw_string(&bchips);
        sprite_draw(&dot1);
        sprite_draw(&dot2);
        render_pass();

        wait_frames(30);

        while(!button_state){
            switch (joystick_state) {
            case JOYSTICK_UP:
                if(round_chips < (total_chips)){
                    round_chips += 1;
                    update_bet_string(round_chips, &bchips_str);
                    update_string(&bchips, &bchips_str, 6);
                }
                break;
            case JOYSTICK_DOWN:
                if(round_chips > 1){
                    round_chips -= 1;
                    update_bet_string(round_chips, &bchips_str);
                    update_string(&bchips, &bchips_str, 6);
                }
                break;
            default:
                break;
            }
            render_pass();
            wait_frames(10);
        }

        erase_string(&t_prompt);
        erase_string(&chips);
        erase_string(&b_prompt);
        erase_string(&bchips);
        sprite_erase(&dot1);
        sprite_erase(&dot2);
        render_pass();

        play_blackjack();
            
            
        if((win == 1) && (double_down == 1)){
            total_chips += (round_chips * 2);
        }
        else if((win == 1) && (double_down == 0)){
            total_chips += round_chips;
        }
        else if((lose == 1) && (double_down == 1)){
            total_chips -= (round_chips * 2);
        }
        else if((lose == 1) && (double_down == 0)){
            total_chips -= round_chips;
        }
    }
}

void update_bet_string(int round_chips, char* bchips_str){
    if(round_chips >= 10000){
        bchips_str[0] = (char)((round_chips / 10000) + 48);
    }
    else{
        bchips_str[0] = (char)32;
    }   
    if(round_chips >= 1000){
        bchips_str[1] = (char)(((round_chips % 10000) / 1000) + 48);
    }
    else{
        bchips_str[1] = (char)32;
    }
    if(round_chips >= 100){
        bchips_str[2] = (char)(((round_chips % 1000) / 100) + 48);
    }
    else{
        bchips_str[2] = (char)32;
    }
    if(round_chips >= 10){
        bchips_str[3] = (char)(((round_chips % 100) / 10) + 48);
    }
    else{
        bchips_str[3] = (char)32;
    }
    bchips_str[4] = (char)((round_chips % 10) + 48);
    bchips_str[5] = '\0';
}