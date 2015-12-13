#include <kilolib.h>

// this program tests the ability of the 'head' of the snake to find and 'eat'
// its prey.

// define movements
typedef enum {
    STRAIGHT,
    LEFT,
    RIGHT,
    STOP
} motion_dir;

// declare variables
uint8_t ftl_id = 0; // kilobot ID for follow-the-leader
motion_dir previous_dir = STOP; // all kilobots are initially not moving

uint8_t new_message = 0; // new message flag
uint8_t sent_message = 0; // sent message flag

uint8_t dist = 100; // distance from head to prey

// declare data structures
message_t prey_msg; // empty message to be sent by prey
distance_measurement_t d_measure; // distance measurement pointer

// simplify updating the motors
void update_motors(motion_dir direction) {
    if (direction != previous_dir) {
        previous_dir = direction;
        switch(direction) {
            case STRAIGHT:
                spinup_motors();
                set_motors(kilo_straight_left, kilo_straight_right);
                break;
            case LEFT:
                spinup_motors();
                set_motors(kilo_turn_left,0);
                break;
            case RIGHT:
                spinup_motors();
                set_motors(0,kilo_turn_right);
                break;
            case STOP:
                set_motors(0,0);
                break;
        }
    }
}

// cause head of snake to navigate towards prey
void snake_head() {
    static uint8_t prev_dist = 100;
    prev_dist = dist;
    
    if (dist <= 30) {
        ftl_id++;
    } else if (dist > prev_dist) {
        if (previous_dir == LEFT) {
            update_motors(RIGHT);
        } else if (previous_dir == RIGHT) {
            update_motors(LEFT);
        }
    }
}

// move head in random direction
void move_randomly() {
    const uint8_t mask = 0b00000011;
    uint8_t rand_dir = rand_soft() & mask;
    
    if (rand_dir == 0 || rand_dir == 3) {
        update_motors(STRAIGHT);
    } else if (rand_dir == 1) {
        update_motors(LEFT);
    } else if (rand_dir == 2) {
        update_motors(RIGHT);
    } else {
        return;
    }
    
    delay(1000);
}

// message receival callback
void message_rx(message_t *m, distance_measurement_t *d) {
    new_message = 1;
    d_measure = *d;
}

// message transmission callback
message_t *message_tx() {
    if (ftl_id == 1) {
        return '\0';
    } else {
        return &prey_msg;
    }
}

void message_tx_success() {
    sent_message = 1;
}

// setup code
void setup() {
    prey_msg.type = NORMAL;
    prey_msg.crc = message_crc(&prey_msg);
    
    // initialize snake head
    if (kilo_uid == 10000) {
        ftl_id = 1;
        set_color(RGB(0,1,0));
    }
}

// main program loop
void loop() {
    if (ftl_id == 1) {
        if (new_message) {
            new_message = 0;
            dist = estimate_distance(&d_measure);
            snake_head();
        } else {
            move_randomly();
        }
    } else if (ftl_id > 1) {
        update_motors(STOP);
        set_color(RGB(0,0,1));
    } else {
        if (sent_message) {
            sent_message = 0;
            
            set_color(RGB(1,0,0));
            delay(100);
            set_color(RGB(0,0,0));
        }
    }
}

int main() {
    // initialize hardware
    kilo_init();
    // register callbacks
    kilo_message_rx = message_rx;
    kilo_message_tx = message_tx;
    kilo_message_tx_success = message_tx_success;
    // start program
    kilo_start(setup, loop);

    return 0;
}
