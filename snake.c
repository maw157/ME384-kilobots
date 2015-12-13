#include <kilolib.h>

uint8_t ftl_id = 0;

uint8_t new_message = 0;
message_t prey_msg;

uint8_t dist = 0;
distance_measurement_t d_measure;

typedef enum {
    LEFT,
    RIGHT,
    STRAIGHT,
    STOP
} DIRECTION;

DIRECTION dir;

// function to simplify updating the motors
void update_motors(uint8_t left, uint8_t right) {
    static uint8_t pleft, pright;
    
    if ((pleft == 0 && left != 0) || (pright == 0 && right != 0)) { spinup_motors(); }
    if ((pleft != left) || (pright != right)) {
        set_motors(left, right);
        pleft = left;
        pright = right;
    }
}

void snake_head() {
    static uint8_t pdist;
    
    if (dist < 10) {
        update_motors(0,0);
        set_color(RGB(0,0,1));
        dir = STOP;
    } else if (dist > pdist) {
        if (dir == LEFT) {
            update_motors(0,kilo_turn_right);
            dir = RIGHT;
        }
        if (dir == RIGHT) {
            update_motors(kilo_turn_left,0);
            dir = LEFT;
        }
    }
}

void init() {
    ftl_id = 1;
    set_color(RGB(0,1,0));
    dir = STOP;
}

void message_rx(message_t *m, distance_measurement_t *d) {
    new_message = 1;
    d_measure = *d;
}

message_t *message_tx() {
    if (ftl_id == 1) {
        return '\0';
    } else {
        return &prey_msg;
    }
}

void setup() {
    prey_msg.type = NORMAL;
    prey_msg.crc = message_crc(&prey_msg);
    
    if (kilo_uid == 10000) {
        init();
    }
}

void loop() {
    if (ftl_id == 1) {
        if (new_message) {
            new_message = 0;
            dist = estimate_distance(&d_measure);
            snake_head();
        } else {
            const mask = 0b00000011;
            uint8_t rand_dir = rand_soft() & mask;
            
            if (rand_dir == 0) {
                update_motors(kilo_straight_left,kilo_straight_right);
                dir = STRAIGHT;
            } else if (rand_dir == 1) {
                update_motors(kilo_turn_left,0);
                dir = LEFT;
            } else if (rand_dir == 2) {
                update_motors(0,kilo_turn_right);
                dir = RIGHT;
            } else {
                return;
            }
        }
    }
}

int main() {
    // initialize hardware
    kilo_init();
    kilo_message_rx = message_rx;
    kilo_message_tx = message_tx;
    // start program
    kilo_start(setup, loop);

    return 0;
}
