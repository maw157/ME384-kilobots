#include <kilolib.h>

// initialize variables
uint8_t ftl_id = 0;         // ID for following the leader: prey = 0, snake = 1, 
                            // and followers increase from snake
uint8_t num_bots = 2;       // keep track of number of bots in snake body
uint8_t is_snake = 0;       // keep track of whether bot is currently snake
uint32_t clock_time = 0;    // stopwatch variable

// set flags
uint8_t new_leader = 0;     // keep track of whether bot is ready after capturing prey
uint8_t new_message = 0;    // message receival flag

// initialize message storage variables
uint8_t transmit_id = 0;        // store ID of received message
uint8_t transmit_leader = 0;    // store new leader state of received message
uint8_t transmit_bots = 2;      // store number of bots of received message
uint8_t transmit_snake = 0;     // store snake state of received message

// initialize data structs
message_t msg;

// message receival callback - store message data and set new message flag
void message_rx(message_t *m, distance_measurement_t *d) {
    transmit_id = m->data[0];
    transmit_leader = m->data[1];
    transmit_bots = m->data[2];
    transmit_snake = m->data[3];
    
    new_message = 1;
}

// message transmission callback - return message
message_t *message_tx(void) {
    return &msg;
}

// update message data for transmission
void update_message() {
    msg.data[0] = ftl_id;
    msg.data[1] = new_leader;
    msg.data[2] = num_bots;
    msg.data[3] = is_snake;
    
    msg.crc = message_crc(&msg);
}

void setup() {
    // use bots with kilo_uids of 384, 1000, 2000
    
    ftl_id = kilo_uid / 1000;
    
    // set LEDs according to place in line
    if (ftl_id == 0) {
        set_color(RGB(1,1,1));
    } else if (ftl_id == 1) {
        is_snake = 1; // set snake flag for line leader
        set_color(RGB(1,0,0));
    } else if (ftl_id == 2) {
        set_color(RGB(0,1,0));
    }
    
    // initialize message
    msg.type = NORMAL;
    update_message();
}

void loop() {
    // check for new message
    if (new_message) {
        // reset new message flag
        new_message = 0;
        
        // if bot is currently the snake
        if (is_snake) {
            // do nothing for two seconds while news of capture propagates
            if (clock_time + 64 > kilo_ticks && new_leader) {
                return;
            // if prey has been found, set new leader flag, increment ID and number
            // of bots, update message, and start stopwatch
            } else if (transmit_id == 0) {
                new_leader = 1;
                ftl_id++;
                num_bots++;
                
                update_message();
                
                clock_time = kilo_ticks;
            // if we have waited for two seconds and the bot behind snake says the
            // rest of the body is ready, clear the new leader flag, disable the snake
            // identifier, and update message
            } else if (transmit_id >= 2 && transmit_leader == 0 && (clock_time + 64 < kilo_ticks)) {
                new_leader = 0;
                is_snake = 0;
                
                update_message();
            }
        // if the bot is prey    
        } else if (ftl_id == 0) {
            // if the message is from the head of the snake (we check both that
            // the bot says it is the snake and that it is the front of the line
            // because the head of the snake doesn't clear its snake status until
            // the body of the snake is ready) then set the new leader flag,
            // increment the ID, update message, and start the stopwatch
            if (transmit_snake && transmit_id == 1) {
                new_leader = 1;
                ftl_id++;
                
                update_message();
                
                clock_time = kilo_uid;
            // if the message is from the head of the snake and we have not
            // waited for two seconds, set the number of bots equal to the
            // number of bots in the message
            } else if ((clock_time + 64 > kilo_ticks) && transmit_snake) {
                num_bots = transmit_bots;
            // if we have waited for two seconds, the previous head of the snake
            // is now part of the body, and the body of the snake is ready, then
            // clear the new leader flag, enable the snake identifier, and update
            // the message
            } else if ((clock_time + 64 < kilo_ticks) && transmit_id == 2 && transmit_leader == 0) {
                new_leader = 0;
                is_snake = 1;
                
                update_message();
            }
        // otherwise the bot is a follower    
        } else {
            // if the new leader flag is not set, we're currently talking to the
            // bot in front, the bot in front says there is a new leader, and
            // the number of bots needs updating, then increment the ID, set the
            // new leader flag, set the number of bots to the transmitted number,
            // update message, and start stopwatch
            if (!new_leader && transmit_id < ftl_id && transmit_leader && num_bots < transmit_bots) {
                ftl_id++;
                new_leader = 1;
                num_bots = transmit_bots;
                
                update_message();
                
                clock_time = kilo_ticks;
            // if the new leader flag is set and we're talking to the bot behind,
            // and we've waited for two seconds and the bot behind has cleared its
            // new leader flag, then clear our new leader flag and update message
            } else if (new_leader && transmit_id > ftl_id && !transmit_leader && (clock_time + 64 > kilo_ticks)) {
                new_leader = 0;
                update_message();
            // if the new leader flag is set, we've waited for two seconds, and
            // we are the last bot in the line, then clear the new leader flag
            // and update message
            } else if (new_leader && (clock_time + 64 < kilo_ticks) && num_bots == ftl_id) {
                new_leader = 0;
                update_message();
            }
        }
    }
    
    // if the bot is not ready (there is a new leader), then
    // blink the LEDs
    if (new_leader) {
        if (ftl_id == 1) {
            set_color(RGB(1,0,0));
            delay(100);
            set_color(RGB(0,0,0));
        } else if (ftl_id == 2) {
            set_color(RGB(0,1,0));
            delay(100);
            set_color(RGB(0,0,0));
        } else if (ftl_id == 3) {
            set_color(RGB(0,0,1));
            delay(100);
            set_color(RGB(0,0,0));
        }
    // otherwise do not blink the LEDs
    } else {
        if (ftl_id == 1) {
            set_color(RGB(1,0,0));
        } else if (ftl_id == 2) {
            set_color(RGB(0,1,0));
        } else if (ftl_id == 3) {
            set_color(RGB(0,0,1));
        }
    }
}

int main() {
    // initialize hardware
    kilo_init();
    // register callbacks
    kilo_message_rx = message_rx;
    kilo_message_tx = message_tx;
    // start program
    kilo_start(setup, loop);

    return 0;
}
