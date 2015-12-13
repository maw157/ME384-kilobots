#include <kilolib.h>

// this program tests an algorithm for following the leader.

// define states
#define WAIT 0
#define MOVE 1

// define movements
typedef enum {
    STRAIGHT,
    LEFT,
    RIGHT,
    STOP
} motion_dir;

// declare variables
uint8_t state = WAIT; // all kilobots start in WAIT mode
uint8_t ftl_id = 0; // ID for following the leader

uint8_t front_id = 0; // store ID of bot in front
uint8_t front_state = 0; // store state of bot in front
uint8_t front_dist = 0; // store distance to bot in front

uint8_t behind_id = 0; // store ID of bot behind
uint8_t behind_state = 0; // store state of bot behind
uint8_t behind_dist = 0; // store distance to bot behind

uint8_t dist = 0; // var to hold calculated distance
uint8_t new_message = 0; // new message flag

//uint32_t clock_time = 0;

// declare structs
message_t msg;
distance_measurement_t d_measure;

// simplify updating the motors
void update_motors(motion_dir direction) {
    static motion_dir previous_dir = STOP;
    
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

// message transmission callback
message_t *message_tx() {
    return &msg;
}
 
// message receival callback
void message_rx(message_t *m, distance_measurement_t *d) {
    // set new message flag and calculate distance
    new_message = 1;
    d_measure = *d;
    dist = estimate_distance(&d_measure);
    
    // store message data for later
    if (m->data[0] == front_id) {
        front_dist = dist;
        front_state = m->data[1];
    } else if (m->data[0] == behind_id) {
        behind_dist = dist;
        behind_state = m->data[1];
    }
}

void update_state() {
    msg.data[1] = state;
    msg.crc = message_crc(&msg);
}

void move() {
    state = MOVE;
    update_state();
    update_motors(STRAIGHT);
    set_color(RGB(0,1,0));
}

void wait() {
    state = WAIT;
    update_state();
    update_motors(STOP);
    set_color(RGB(1,0,0));
}

void setup() {
    ftl_id = kilo_uid / 1000;
    
    // initialize message with bot's ID
    msg.type = NORMAL;
    msg.data[0] = ftl_id;
    msg.crc = message_crc(&msg);
    
    front_id = ftl_id - 1;
    behind_id = ftl_id + 1;
    
    // odd-numbered bots move first
    if (ftl_id % 2 == 1) {
        move();
    } else {
        wait();
    }
}

void loop() {
    if (new_message) {
        new_message = 0;
        
        if (ftl_id == 1) {
            if (behind_state == WAIT && behind_dist < 40) {
                move();
            } else {
                wait();
            }
        } else if (ftl_id == 3) {
            if (front_state == WAIT && front_dist > 40) {
                move();
            } else {
                wait();
            }
        } else {
            if (front_state == WAIT && behind_state == WAIT && front_dist > 40) {
                move();
            } else {
                wait();
            }
        }
    }
}

int main() {
    // initialize hardware
    kilo_init();
    // initialize callbacks
    kilo_message_tx = message_tx;
    kilo_message_rx = message_rx;
    // start program
    kilo_start(setup, loop);

    return 0;
}
