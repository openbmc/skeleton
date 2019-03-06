#ifndef __OBJECTS_GPIO_UTILITIES_H__
#define __OBJECTS_GPIO_UTILITIES_H__

#include <stdint.h>
#include <gio/gio.h>
#include <stdbool.h>

typedef struct {
  gchar* name;
  gchar* dev;
  uint16_t num;
  uint16_t chip_id;
  gchar* direction;
  int fd;
  bool irq_inited;
} GPIO;


//gpio functions
#define GPIO_OK           0x00
#define GPIO_ERROR        0x01
#define GPIO_OPEN_ERROR   0x02
#define GPIO_INIT_ERROR   0x04
#define GPIO_READ_ERROR   0x08
#define GPIO_WRITE_ERROR  0x10
#define GPIO_LOOKUP_ERROR 0x20

void gpio_close(GPIO*);
int  gpio_open(GPIO*, uint8_t);
int gpio_write(GPIO*, uint8_t);
int gpio_get_params(GPIO*);
int gpio_read(GPIO*,uint8_t*);
void gpio_inits_done();

#endif
