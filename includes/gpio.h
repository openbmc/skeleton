#ifndef __OBJECTS_GPIO_UTILITIES_H__
#define __OBJECTS_GPIO_UTILITIES_H__

#include <stdint.h>
#include <gio/gio.h>

typedef struct {
  gchar* name;
  gchar* dev;
  uint16_t num;
  gchar* direction;
  int fd;
} GPIO;


//gpio functions
#define GPIO_OK 0
#define GPIO_ERROR -1
#define GPIO_OPEN_ERROR -2
#define GPIO_INIT_ERROR -3
#define GPIO_READ_ERROR -4
#define GPIO_WRITE_ERROR -5
#define GPIO_LOOKUP_ERROR -6

int gpio_init(GDBusConnection*, GPIO*);
void gpio_close(GPIO*);
int  gpio_open(GPIO*);
int gpio_write(GPIO*, uint8_t);
int gpio_writec(GPIO*, char);
int gpio_clock_cycle(GPIO*, int);
int gpio_read(GPIO*,uint8_t*);

#endif
