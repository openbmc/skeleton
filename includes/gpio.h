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
void gpio_init(GDBusConnection*, GPIO*);
void gpio_close(GPIO*);
int  gpio_open(GPIO*);
void gpio_write(GPIO*, uint8_t);
void gpio_writec(GPIO*, char);
void gpio_clock_cycle(GPIO*, int);
uint8_t gpio_read(GPIO*);

#endif
