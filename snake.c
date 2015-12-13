#include <kilolib.h>

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
uint8_t state = WAIT;       // all kilobots start in WAIT mode
uint8_t ftl_id = 0;         // ID for following the leader

uint8_t front_id = 0;       // store ID of bot in front
uint8_t front_state = 0;    // store state of bot in front
uint8_t front_dist = 0;     // store distance to bot in front

uint8_t behind_id = 0;      // store ID of bot behind
uint8_t behind_state = 0;   // store state of bot behind
uint8_t behind_dist = 0;    // store distance to bot behind

uint8_t dist = 0;           // var to hold calculated distance
uint32_t count = 0;         // stopwatch variable

uint8_t new_message = 0;    // new message flag
uint8_t sent_message = 0;   // sent message flag

uint8_t found = 0;          // found bot flag
uint8_t reset_flag = 0;     // reset flag

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

// successfull message transmission callback
void message_tx_success() {
    sent_message = 1;
}
 
// message receival callback
void message_rx(message_t *m, distance_measurement_t *d) {
    // set new message flag and calculate distance
    new_message = 1;
    d_measure = *d;
    dist = estimate_distance(&d_measure);
    
    // store message data for later
    if (m->data[0] == front_id) {
        if (front_id == 0 && dist < 40) {
            found = 1;
        }
        
        front_dist = dist;
        front_state = m->data[1];
        reset_flag = m->data[2];
    } else if (m->data[0] == behind_id) {
        if (behind_id == 1 && dist < 40) {
            found = 1;
        }
        
        behind_dist = dist;
        behind_state = m->data[1];
    }
}

void update_message() {
    msg.data[0] = ftl_id;
    msg.data[1] = state;
    msg.data[2] = reset_flag;
    msg.crc = message_crc(&msg);
}

void move() {
    state = MOVE;
    update_message();
    update_motors(STRAIGHT);
    set_color(RGB(0,1,0));
}

void wait() {
    state = WAIT;
    update_message();
    update_motors(STOP);
    set_color(RGB(1,1,0));
}

void follow() {
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

void reset() {
    reset_flag = 0;
    ftl_id++;
    update_message();
    
    front_id = ftl_id - 1;
    behind_id = ftl_id + 1;
    
    if (ftl_id % 2 == 1) {
        move();
    } else {
        wait();
    }
}

void setup() {
    if (kilo_uid == 10000) {
        ftl_id = 1;
    }
    
    // initialize message
    msg.type = NORMAL;
    update_message();
    
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
    if (count + 120 > kilo_ticks) {
        return;
    }
    
    if (ftl_id == 0) {
        if (sent_message) {
            sent_message = 0;
            
            set_color(RGB(1,1,1));
            delay(100);
            set_color(RGB(0,0,0));
        }
        
        if (new_message) {
            new_message = 0;
            
            if (reset_flag && count + 90 < kilo_ticks) {
                found = 0;
                reset();
            } else if (found) {
                reset_flag = 1;
                update_message();
                count = kilo_ticks;
            }
        }
    } else if (ftl_id == 1) {
        if (new_message) {
            new_message = 0;
            
            if (reset_flag && count + 90 < kilo_ticks) {
                found = 0;
                reset();
            } else if (found) {
                update_motors(STOP);
                reset_flag = 1;
                update_message();
                count = kilo_ticks;
            } else {
                follow();
            }
        } else if (kilo_uid == 10000) {
            update_motors(STRAIGHT);
        }
    } else {
        if (new_message) {
            new_message = 0;
            
            if (reset_flag && count + 90 < kilo_ticks) {
                reset();
            } else if (reset_flag) {
                update_motors(STOP);
                update_message();
                count = kilo_ticks;
            } else {
                follow();
            }
        }
    }
}

int main() {
    // initialize hardware
    kilo_init();
    // initialize callbacks
    kilo_message_tx = message_tx;
    kilo_message_tx_success = message_tx_success;
    kilo_message_rx = message_rx;
    // start program
    kilo_start(setup, loop);

    return 0;
}