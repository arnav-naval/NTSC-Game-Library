/******************************************************************************

blackjack game logic, print functions will become obsolete so it will shorten
written by Riley Mann

*******************************************************************************/
#include "stm32f0xx.h"
#include <stdio.h>

static uint32_t seed = 0;
//0 is ace, 10 is jack, 11 is queen, 12 is king
//0-12 is clubs, 13-25 is diamonds, 26-38 is hearts, 39-51 is spades
int used_cards[52] = {0}; //an array where cards already used are placed so the random card function can check against it
int used_cards_count = 0; //number of cards drawn (deck is shuffled after 52 cards drawn)
int card_number;
int win = 0; // true if win
int lose = 0;  //true if lose
int double_down = 0; //true if user chose to double down
int tie = 0; //true if there is a tie

// a pseudo random number generator utilizing systick
void getSeed(){
    seed = SysTick->VAL;
}

uint32_t getRandomNumber(void){
    //a and b can be anything
    //to do: check to make sure you're not getting the same random numbers each time
    int a = 12345;
    int b = 6789;
    seed = a * seed + b;
    return (int)(seed % 52);
}

//function to shuffle the deck after all cards are drawn
void reset_cards(){
    used_cards_count = 0;
    for(int i = 0; i < 52; i++){
        used_cards[i] = -1;
    }
}

//draws a unique card every time, resets each time the code is run
int draw_card(){
    int card;
    if(used_cards_count >= 52){
        reset_cards();
    }
    while(1){
        getSeed();
        card = getRandomNumber();
        int is_used = 0;
        for(int i = 0; i < used_cards_count; i++){
            if(used_cards[i] == card){
                is_used = 1;
                break;
            }
        }
        if(!is_used){
            used_cards[used_cards_count] = card;
            used_cards_count++;
            return card;
        }
    }
}

int stay_or_hit(){
    int choice;
    printf("type 1 for hit, 0 for stay, 2 for double down: ");
    scanf("%d", &choice);
    if((choice == 1) && (double_down == 0)){
        return(draw_card());
    }
    if(choice == 2){
        double_down = 1;
        return(draw_card());
    }
    else{
        return -1;
    }
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

int play_blackjack(){
    int player_ace = 0; //equal to the number of aces in the players hand
    int dealer_ace = 0; //the same thing for the dealer
    int bust = 0; //becomes true if the hand is a bust
    
    //drawing initial cards and checking if any are aces
    int player_first_card = draw_card(); //face up
    player_ace += ace_check(player_first_card); //check if its an ace, add to ace counter
    int dealer_first_card = draw_card(); //face down
    dealer_ace += ace_check(dealer_first_card);
    int player_second_card = draw_card(); //face up
    player_ace += ace_check(player_second_card);
    int dealer_second_card = draw_card(); //face up
    dealer_ace += ace_check(dealer_second_card);
    
    //display player first card
    //display player second card
    //display dealer first card face down
    //display dealer second card
    
    //calculating initial totals
    int player_total = calculate_card_points(player_first_card) + calculate_card_points(player_second_card);
    int dealer_total = calculate_card_points(dealer_first_card) + calculate_card_points(dealer_second_card);
    
    //hit or stand functions
    while(1){
        if((player_ace == 1) && (player_total == 10)){
            break;
        }
        //the variable choice is the number for the card you draw
        int choice = stay_or_hit(); //return -1 for stay
        if(choice == -1){
            break;
            //game is over see if you won
        }
        else{
            player_ace += ace_check(choice);
            player_total += calculate_card_points(choice);
           //display the new card the player drew
        }
        //assumes all the aces are 1 to check for a bust
        if(player_total + player_ace > 21){
           //display "bust" message
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
    while(!(((dealer_total + dealer_ace) > 16) || ((dealer_ace >= 1) && ((dealer_total + dealer_ace) > 6)))){
        if((dealer_ace == 1) && (dealer_total == 10)){
            break;
        }
        //automatically draws card if the while condition is true
        int choice = draw_card();
        dealer_ace += ace_check(choice);
        dealer_total += calculate_card_points(choice);
       //display the new card the dealer drew
        //assumes all the aces are 1 to check for a bust
        if(dealer_total + dealer_ace > 21){
        //    printf("dealer bust!\n");
            dealer_bust = 1;
            break;
        }
    }
    //add dealers aces to the final total
    if(dealer_ace != 0){
        dealer_total = choose_ace_value(dealer_ace, dealer_total);
    }
    
    if(bust == 0 && dealer_bust == 0){
       //display the dealers first card
       //display the players total
       //display the dealers total 
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
            tie = 1;
            win = 0;
            lose = 0;
           //print both player and dealer busted, tie
        }
        else if(dealer_bust == 1 && bust == 0){
       //print "you win" message
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
    return 0;
}


int main(){
    int total_chips = 10; //the total number of chips the player is working with
    int round_chips; //the chips the player chooses to bet that round 
    while(total_chips != 0){
        //ask player how many total chips they would like to bet;
        round_chips = 1;
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
        //print the players new total number of chips
    }
    return 0;
}

