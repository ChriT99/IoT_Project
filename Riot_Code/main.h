#ifndef FUNCTIONS
#define FUNCTIONS

static void *emcute_thread(void *arg);
static int get_connection(void);
static int publish(char* msg);
static int subscribe(void);
void echo_cb(void* arg);
int read_distance(void);
int detect_motion(void);
void check_bin_level(void);
void init_sensors(void);
void mqtt_configuration(void);

#endif 