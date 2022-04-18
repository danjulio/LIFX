/************************************************************************/
/* Contains a data structure encapsulating LIFX product IDs and         */
/* capabilities as of Jan 2022.                                         */
/*                                                                      */
/* Written by Dan Julio to extend the Lifx library.                     */
/************************************************************************/
#ifndef _LIFX_PRODUCTS_
#define _LIFX_PRODUCTS_
#include <stdint.h>
#include <stdbool.h>


extern "C" {

// Light type (color capable, white with kelvin adjustment, white with
// brightness adjust, on/off-only relay)
typedef enum {COLOR, ADJ_WHITE, WHITE, RELAY} lifx_light_type;

// Zone types
typedef enum {SINGLE, LINEAR, EXT_LINEAR, MATRIX, MATRIX_CHAIN} lifx_zone_type;

// Table data
typedef struct {
  char* name;
  uint32_t vendor;
  uint32_t pid;
  lifx_light_type color;
  bool infrared;
  lifx_zone_type zones;
  bool hev;
} lifx_types_struct;


//
// Global Variables
//
extern const lifx_types_struct lifx_types[];


//
// API
//
int lifx_find_pid_index(uint32_t pid);

}


#endif // _LIFX_PRODUCTS_