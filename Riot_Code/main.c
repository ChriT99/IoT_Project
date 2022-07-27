#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"
#include "net/netopt.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "servo.h"
#include "cpu.h"
#include "board.h"
#include "periph/pwm.h"

#include "pir.h"

#include "main.h"


//macros for servo
/*#define SERVO_MIN               550
#define SERVO_MAX               2500
#define DEGREE_MAX              180
#define DEGREE_TO_US(x) ((x*(SERVO_MAX-SERVO_MIN)/DEGREE_MAX)+SERVO_MIN)
#define SERVO_CHANNEL           1
#define SERVO_PWM               0
*/
//servo
#define DEV         PWM_DEV(0)
#define CHANNEL     1
#define SERVO_MIN        (1000U)
#define SERVO_MAX        (2000U)

#define 	PIR_PARAM_GPIO   GPIO_PIN(PORT_B,4)

//macros for bin
#define BIN_THRESHOLD                5

//macros for mqtt
#define EMCUTE_PRIO         (THREAD_PRIORITY_MAIN - 1)
#define _IPV6_DEFAULT_PREFIX_LEN        (64U)
#define NUMOFSUBS           (16U)
#define TOPIC_MAXLEN        (64U)


#define BROKER_PORT         1885
#define BROKER_ADDRESS      "fec0:affe::1"

#define DEFAULT_INTERFACE   ("4")
#define DEVICE_IP_ADDRESS   ("fec0:affe::99")
#define TOPIC_NAME_PUB      "topic_pub"
#define TOPIC_NAME_SUB      "topic_sub"


/*servo motor definition
typedef struct servo_device_s {
    servo_t servo;
    int degree;
    uint8_t manual_override;

}servo_device_t;

servo_device_t my_servo;*/

//servo
static servo_t servo;

//if bin_cover=0 -> closed, otherwise -> open
static int bin_cover=0;
//if = 0 bin empty otherwise full
int full = 0;

extern int _gnrc_netif_config(int argc, char **argv);

//ultrasonic sensor definition
gpio_t trigger_pin = GPIO_PIN(PORT_B, 3); //D3 
gpio_t echo_pin = GPIO_PIN(PORT_B, 5); //D4 
uint32_t echo_time;
uint32_t echo_time_start;

//pir sensor definition
pir_params_t pir_parameters;
pir_t dev;

//led definition
gpio_t led_pin = GPIO_PIN(PORT_B, 10); //D7


//mqtt
static char stack[THREAD_STACKSIZE_DEFAULT];
static msg_t queue[8];
static emcute_sub_t subscriptions[NUMOFSUBS];
static char topics[NUMOFSUBS][TOPIC_MAXLEN];


// configuration of mqtt

static void *emcute_thread(void *arg){
    (void)arg;
    emcute_run(BROKER_PORT, "stm32");
    return NULL;
}



static int publish(char* msg){
    emcute_topic_t topic;
    unsigned flags = EMCUTE_QOS_0;
    topic.name = TOPIC_NAME_PUB;

    emcute_reg(&topic);

    emcute_pub(&topic, msg, strlen(msg), flags);

    return 0;
}

static int subscribe(void){

    unsigned flags = EMCUTE_QOS_0;
    subscriptions[0].cb = on_pub;
    strcpy(topics[0], TOPIC_NAME_SUB);
    subscriptions[0].topic.name = topics[0];
    
    emcute_sub(&subscriptions[0], flags);

    return 0;
}

static void on_pub(const emcute_topic_t *topic, void *data, size_t len){
    (void)topic;

    char *in = (char *)data;
    char msg[len+1];
    strncpy(msg, in, len);
    msg[len] = '\0';
    if (strcmp(msg, "open") == 0){
        if(bin_cover==0){
            //open bin cover
            servo_set(&servo, SERVO_MIN);
            bin_cover=1;
            }
    }
    else if (strcmp(msg, "close") == 0){
        if(bin_cover==1){
            //close bin cover
            servo_set(&servo, SERVO_MAX);
            bin_cover=0;
        }
    }
    puts("");

}

static int get_connection(void){
    sock_udp_ep_t gw = { .family = AF_INET6, .port = BROKER_PORT };
    char *topic = NULL;
    char *message = NULL;
    size_t len = 0;

    ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, BROKER_ADDRESS);

    if (emcute_con(&gw, true, topic, message, len, 0) != EMCUTE_OK) {
        printf("error: unable to connect to [%s]:%i\n", BROKER_ADDRESS, BROKER_PORT);
        return 1;
    }
    printf("Successfully connected to gateway at [%s]:%i\n",BROKER_ADDRESS, BROKER_PORT);

    return 0;
}

static int ipv6_addr_add(char *ip_address){

    char * arg[] = {"ifconfig", "4", "add", ip_address};
    return _gnrc_netif_config(4, arg);
}


//callback function ultrasonic
void echo_cb(void* arg){ 
	int val = gpio_read(echo_pin);
	uint32_t echo_time_stop;

    (void) arg;

	if(val){
		echo_time_start = xtimer_now_usec();
	}
    else{
		echo_time_stop = xtimer_now_usec();
		echo_time = echo_time_stop - echo_time_start;
	}
}

//ultrasonic sensor read distance
int read_distance(void){ 
	
	uint32_t dist;
    dist=0;
    echo_time = 0;

    gpio_clear(trigger_pin);
    xtimer_usleep(20); 
    gpio_set(trigger_pin);

    xtimer_msleep(100); 

    if(echo_time > 0){
        dist = echo_time/58;
    }
	
    return dist;
}

//detect motion pir
int detect_motion(void){ 
    int motion = 0;
    if (pir_get_status(&dev) != PIR_STATUS_ACTIVE){
        printf("Motion is  detected \n"); 
        motion =1;
        
    }
    else{
        printf("Motion is not detected\n");
    }
    return motion;
}

//check bin level
int check_bin_level(void){ 
   
    int distance = read_distance(); //50 bin size, >40 bin is empty, <5 means full, 
    char msg[4];
    sprintf(msg, "%d", distance);
    publish(msg);
    if ((full=0) && (distance <= BIN_THRESHOLD) ){
        gpio_set(led_pin);
        full =1;
    }
    else if ((full=1) && (distance > BIN_THRESHOLD)){
        gpio_clear(led_pin);
        full =0;
    }else{
        printf("error check bin level");
    }
    return full;
}

//initialization of sensors
void init_sensors(void){ 
    
    //ultrasonic sensor
    gpio_init(trigger_pin, GPIO_OUT);
    gpio_init_int(echo_pin, GPIO_IN, GPIO_BOTH, &echo_cb, NULL);

    /*servo init
    servo_t *servo = &(my_servo.servo);
    my_servo.degree = 0;
    my_servo.manual_override = 0;
    int ret = servo_init(servo, PWM_DEV(SERVO_PWM), SERVO_CHANNEL, SERVO_MIN, SERVO_MAX);
    if ( ret < 0 ){
        printf("Failed to init servo motor");
    }else{
        // Only if we initialize the servo, set the position to 0.
        servo_set(servo, DEGREE_TO_US(0));
    }*/

    //pir init
     if (pir_init(&dev, &pir_parameters) == PIR_OK){
        printf("PIR sensor connected\n");
    }
    else{
    printf("Failed to connect to PIR sensor\n");
    }

    //init led
    if (gpio_init(led_pin, GPIO_OUT)) {
        printf("Error to initialize GPIO_PIN(%d %d)\n", PORT_B, 10);
    }

    //servo init
    servo_init(&servo, DEV, CHANNEL, SERVO_MIN, SERVO_MAX);
    servo_set(&servo, SERVO_MAX);

    
}

void mqtts_configuration(void){ //initializes the connection with the MQTT-SN broker
    msg_init_queue(queue, ARRAY_SIZE(queue));
    memset(subscriptions, 0, (1 * sizeof(emcute_sub_t)));
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0, emcute_thread, NULL, "emcute");
    
    ipv6_addr_add(DEVICE_IP_ADDRESS);

    get_connection();
    subscribe();
}

int main(void){
    xtimer_sleep(5);
    init_sensors();
    mqtts_configuration();
    

    while(true){
        int motion = detect_motion();
        printf("value motion = %d \n",motion);

        if(motion==1 && bin_cover==0){
            //open servo
            servo_set(&servo, SERVO_MIN);
            bin_cover=1;
        }else{
            printf("error to open bin");
        }
        xtimer_sleep(5);
        servo_set(&servo, SERVO_MAX);
        bin_cover=0;

        if(check_bin_level()==1){
            servo_set(&servo, SERVO_MIN);
            bin_cover=1;
        }
        xtimer_sleep(5);
    }  

 return 0;
}