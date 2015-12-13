#include <kilolib.h>

// initialize variables
uint8_t ftl_id = 0;         // ID for following the leader: prey = 0, snake = 1, 
                            // and followers increase from snake
uint8_t is_snake = 0;       // keep track of whether bot is currently snake

// set flags
uint8_t new_message = 0;    // message receival flag

// initialize message storage variables
uint8_t transmit_id = 0;        // store ID of received message
uint8_t transmit_snake = 0;     // store snake state of received message

// initialize data structs
message_t msg;

// message receival callback - store message data and set new message flag
void message_rx(message_t *m, distance_measurement_t *d) {
    transmit_id = m->data[0];
    transmit_snake = m->data[1];
    
    new_message = 1;
}

// message transmission callback - return message
message_t *message_tx(void) {
    return &msg;
}

// update message data for transmission
void update_message() {
    msg.data[0] = ftl_id;
    msg.data[1] = is_snake;
    
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
        if (is_snake && ftl_id == 1) {
            // if prey has been found, set new leader flag, increment ID and number
            // of bots, update message, and start stopwatch
            if (transmit_id == 0) {
                ftl_id++;
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
                ftl_id++;
                is_snake = 1;
                update_message();
            }
        // otherwise the bot is a follower    
        } else {
            if (transmit_id == ftl_id) {
                ftl_id++;
                update_message();
            }
        }
    }
    
    if (ftl_id == 1) {
        set_color(RGB(1,0,0));
    } else if (ftl_id == 2) {
        set_color(RGB(0,1,0));
    } else if (ftl_id == 3) {
        set_color(RGB(0,0,1));
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
