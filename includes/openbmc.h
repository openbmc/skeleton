#ifndef __OPENBMC_H__
#define __OPENBMC_H__

#include <stdint.h>

// Macros
#define GET_VARIANT(v)         g_variant_get_variant(v) 
#define GET_VARIANT_D(v)       g_variant_get_double(g_variant_get_variant(v))
#define GET_VARIANT_U(v)       g_variant_get_uint32(g_variant_get_variant(v))
#define GET_VARIANT_B(v)       g_variant_get_byte(g_variant_get_variant(v))
#define NEW_VARIANT_D(v)       g_variant_new_variant(g_variant_new_double(v)) 
#define NEW_VARIANT_U(v)       g_variant_new_variant(g_variant_new_uint32(v)) 
#define NEW_VARIANT_B(v)       g_variant_new_variant(g_variant_new_byte(v)) 
#define VARIANT_COMPARE(x,y)   g_variant_compare(GET_VARIANT(x),GET_VARIANT(y))



#ifdef __arm__
static inline void write_reg(uint32_t val,void* addr)
{
        asm volatile("" : : : "memory");
        *(volatile uint32_t *)addr = val;
}
static inline devmem(uint32_t val, uint32_t reg)
{
	void* r = (void*)reg;
       write_reg(val,r);
}
#else
static inline devmem(uint32_t val, uint32_t reg)
{

}
#endif

typedef struct {
	gint argc;
	gchar **argv;
	GMainLoop *loop;	

} cmdline;



#endif
