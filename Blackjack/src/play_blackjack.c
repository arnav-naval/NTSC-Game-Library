/******************************************************************************

this file runs once for each round of blackjack played, the variables win, lose,
tie, and double down are updated with the results of the hand

*******************************************************************************/
#include "stm32f0xx.h"
#include <stdio.h>


int used_cards[52]; //an array where cards already used are placed so the random card function can check against it
int used_cards_count; //number of cards drawn (deck is shuffled after 52 cards drawn)
int card_number;
int win; // true if win
int lose;  //true if lose
int double_down; //true if user chose to double down
int tie; //true if there is a tie
uint32_t joystickX; //hold joystick position on X axis
uint32_t joystickY; //hold joystick position on Y axis
const char *card_string; //FOR DISPLAY PURPOSES DELETE

void TIM15_IRQHandler(){
    TIM15->SR &= ~TIM_SR_UIF;
    if(TIM15->CNT >= 1000000){
        TIM15->CNT = 0;
    }
}

void init_tim15(void){
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
    TIM15->PSC = 4800 - 1;
    TIM15->ARR = 10000 - 1;
    TIM15->DIER |= TIM_DIER_UIE;
    TIM15->CR1 |= TIM_CR1_CEN;
    NVIC_EnableIRQ(TIM15_IRQn);
}

//produces a random integer 0-51
int getRandomNumber(){
    int seed = (int)TIM15->CNT;
    int random_number = seed * 5483 + 32891 * 4326 / 430;
    random_number = random_number % 52;
    return random_number;
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

int stand_or_hit(){
    int choice =0; //choice = 1 for hit, 2 for double down, 0 for stand
    //default option is stand if player hits button without moving joystick
    spi1_display1("lft hit rgt std up dd");
    
    while(readpin(0) == 0){
        if((joystickX >= 1700 && joystickX <= 2300) && (joystickY <= 1000)){
            choice = 2; //double down
            spi1_display2("double down");
        }
        if((joystickX <= 1000) && (joystickY >= 1700 && joystickY <= 2300)){
            choice = 1; //hit
            spi1_display2("hit           ");
        }
        if((joystickX >= 3000) && (joystickY >= 1700 && joystickY <= 2300)){
            choice = 0;
            spi1_display2("stand        ");
        }
    }
    spi1_display1("selected         ");
    
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
   return 0;
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

    nano_wait(10000000);
    spi1_display1("starting game...");
    spi1_display2("                  ");


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
    nano_wait(10000000000000000);
    spi1_display2("               ");
    spi1_display1("P C1:          ");
    card_string = int_to_string(player_first_card);
    spi1_display2(card_string);

    //display player second card
    nano_wait(10000000000000000);
    spi1_display2("               ");
    spi1_display1("P C2:          ");
    card_string = int_to_string(player_second_card);
    spi1_display2(card_string);

    //display dealer first card face down
    nano_wait(10000000000000000);
    spi1_display2("               ");
    spi1_display1("D C1:          ");
    spi1_display2("FD            ");

    //display dealer second card
    nano_wait(100000000000000000);
    spi1_display2("               ");
    spi1_display1("D C2:          ");
    card_string = int_to_string(dealer_second_card);
    spi1_display2(card_string);
    nano_wait(100000000000000000);
    
    
    //calculating initial totals
    int player_total = calculate_card_points(player_first_card) + calculate_card_points(player_second_card);
    int dealer_total = calculate_card_points(dealer_first_card) + calculate_card_points(dealer_second_card);
    
    //hit or stand functions
    while(1){
        if((player_ace == 1) && (player_total == 10)){
            break;
        }
        //the variable choice is the number for the card you draw
        int choice = stand_or_hit(); //return -1 for stand
        if(choice == -1){
            break;
            //game is over see if you won
        }
        else{
            //DISPLAY THE NEW CARD DRAWN BY PLAYER
            nano_wait(10000000000000000);
            spi1_display2("               ");
            spi1_display1("new card:          ");
            card_string = int_to_string(choice);
            spi1_display2(card_string);
            nano_wait(10000000000000000);

            player_ace += ace_check(choice);
            player_total += calculate_card_points(choice);
        }
        //assumes all the aces are 1 to check for a bust
        if(player_total + player_ace > 21){
            //DISPLAY BUST MESSAGE
            nano_wait(10000000000000000);
            spi1_display1("bust!     ");
            spi1_display2("               ");
            nano_wait(10000000000000000);
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
            //DISPLAY DEALER BUSTS MESSAGE
            nano_wait(10000000000000000);
            spi1_display1("dealer bust!    ");
            spi1_display2("               ");
            nano_wait(10000000000000000);
            dealer_bust = 1;
            break;
        }
    }
    //add dealers aces to the final total
    if(dealer_ace != 0){
        dealer_total = choose_ace_value(dealer_ace, dealer_total);
    }
    
    if(bust == 0 && dealer_bust == 0){
        //DISPLAY DEALER'S FIRST CARD
        nano_wait(10000000000000000);
        spi1_display2("               ");
        spi1_display1("D C1:          ");
        card_string = int_to_string(dealer_first_card);
        spi1_display2(card_string);
        nano_wait(10000000000000000);

        //DISPLAY DEALER'S TOTAL
        nano_wait(10000000000000000);
        spi1_display2("               ");
        spi1_display1("dealer total:      ");
        card_string = int_to_string(dealer_total);
        spi1_display2(card_string);
        nano_wait(10000000000000000);

        //DISPLAY PLAYER'S TOTAL
        nano_wait(10000000000000000);
        spi1_display2("               ");
        spi1_display1("player's total:      ");
        card_string = int_to_string(player_total);
        spi1_display2(card_string);
        nano_wait(10000000000000000);



        if((player_total > dealer_total) && (player_total <= 21)){
         //print "you win" message
/*
            nano_wait(10000000000000000);
            spi1_display1("you win!       ");
            spi1_display2("               ");
            nano_wait(10000000000000000);
            */
            win = 1;
            lose = 0;
            tie = 0;
            play_happy_sound();
        }
        else if(player_total < dealer_total){
           //print "you lose" message
/*
            nano_wait(10000000000000000);
            spi1_display1("you lose :(    ");
            spi1_display2("               ");
            nano_wait(10000000000000000);
            */
            lose = 1;
            win = 0;
            tie = 0;
            play_sad_sound();
        }
        else{
           //print "tie" message
/*
            nano_wait(10000000000000000);
            spi1_display1("tie            ");
            spi1_display2("               ");
            nano_wait(10000000000000000);
            */
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
/*
            nano_wait(10000000000000000);
            spi1_display1("tie            ");
            spi1_display2("both busted    ");
            nano_wait(10000000000000000);
            */
        }
        else if(dealer_bust == 1 && bust == 0){
       //print "you win" message
/*
            nano_wait(10000000000000000);
            spi1_display1("you win!       ");
            spi1_display2("               ");
            nano_wait(10000000000000000);
            */
            win = 1;
            lose = 0;
            tie = 0;
            play_happy_sound();
        }
        else{
            lose = 1;
            win = 0;
            tie = 0;
            play_sad_sound();
        }
    }
    
    return 0;
}

//FUNCTIONS TO SUPPORT SAVING HIGH SCORES
uint8_t name[3] = {0, 0, 0}; //numbers 0-25 represent A-Z
const char* name_string; //JUST FOR OLED CAN DELETE

uint32_t get_name(){
    uint32_t name_32bit;
    int i = 0;
    while(1){
        //UPDATE DISPLAY OF LETTERS
        name_string = int_to_string(name[i]);
        spi1_display2(name_string);
        if(joystickY <= 1000){
            if(name[i] <= 24){
                name[i] += 1;
                nano_wait(100000000);
                spi1_display2(name_string);
            }
        }
        if(joystickY >= 3000){
            if(name[i] >= 1){
                name[i] -= 1;
                nano_wait(100000000);
                spi1_display2(name_string);
            }
        }
        if(readpin(0) != 0){
            name[i] = name[i] & 0x1F;
            name_32bit = name_32bit | (name[i] << (27 - (i * 5)));
            i += 1;
            nano_wait(500000000);
        }
        if(i >= 3){
            break;
        }
    }
    name_32bit &= ~0x1FFFF;
    // adc_to_string(name_32bit, &name_string);
    // spi1_display2(name_string);
    return name_32bit;
    //name_32 bit is returning a value where the first 15 bits represent the 3 letters and every other bit is set to 1
}

int check_if_highscore(uint32_t high_score, uint32_t final_score){
    uint32_t score_mask = 0x1FFFF;
    uint32_t score = high_score & score_mask;
    if(final_score > high_score){
        return 1;
    }
    else{
        return 0;
    }
}

void highscore(int total_chips){
//create a uint32_t, 5 bits for each number, 17 for total score, save it to eeprom
    uint32_t final_score = (uint32_t)total_chips;
    //SCORES FROM THE EEPROM
    uint32_t high_scores[5] = {0,0,0,0,0}; //THE EEPROM VALUES WILL GO HERE;
    int i = 0;
    while(i < 4){
        int check = check_if_highscore(high_scores[i], final_score);
        //if the final score is higher than one of the high scores
        if(check == 1){
            //get the name from the player, put the ascii values of the 3 letters into final_score
            uint32_t name_32bit = get_name();
            final_score |= name_32bit;
            high_scores[i] = final_score;
            //save final_score in high_scores[i]
            //save high_scores array to eeprom
            break;
        }
        else{
            i += 1;
        }
    }
    // adc_to_string(high_scores[2], &name_string);
    // spi1_display2(name_string);
    
    //read each uint32_t from eeprom & with score mask, if final_score > any eeprom score, run get name, add top 15 digits to final_score, replace value;
    //display in numerical order
    //resave to eeprom
}
