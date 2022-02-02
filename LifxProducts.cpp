/************************************************************************/
/* Contains a data structure encapsulating LIFX product IDs and         */
/* capabilities as of Jan 2022.                                         */
/*                                                                      */
/* Written by Dan Julio to extend the Lifx library.                     */
/************************************************************************/
#include "LifxProducts.h"


extern "C" {
 
const lifx_types_struct lifx_types[] = {
  {"LIFX Original 1000", 1, 1, COLOR, false, SINGLE, false},
  {"LIFX Color 650", 1, 3, COLOR, false, SINGLE, false},
  {"LIFX White 800 (LV)", 1, 10, ADJ_WHITE, false, SINGLE, false},
  {"LIFX White 800 (HV)", 1, 11, ADJ_WHITE, false, SINGLE, false},
  {"LIFX Color 1000", 1, 15, COLOR, false, SINGLE, false},
  {"LIFX White 900 BR30 (LV)", 1, 18, ADJ_WHITE, false, SINGLE, false},
  {"LIFX White 900 BR30 (HV)", 1, 19, ADJ_WHITE, false, SINGLE, false},
  {"LIFX Color 1000 BR30", 1, 20, COLOR, false, SINGLE, false},
  {"LIFX Color 1000", 1, 22, COLOR, false, SINGLE, false},
  {"LIFX A19", 1, 27, COLOR, false, SINGLE, false},
  {"LIFX BR30", 1, 28, COLOR, false, SINGLE, false},
  {"LIFX A19 Night Vision", 1, 29, COLOR, true, SINGLE, false},
  {"LIFX BR30 Night Vision", 1, 30, COLOR, true, SINGLE, false},
  {"LIFX Z", 1, 31, COLOR, false, LINEAR, false},
  {"LIFX Z", 1, 32, COLOR, false, EXT_LINEAR, false},
  {"LIFX Downlight", 1, 36, COLOR, false, SINGLE, false},
  {"LIFX Downlight", 1, 37, COLOR, false, SINGLE, false},
  {"LIFX Beam", 1, 38, COLOR, false, LINEAR, false},
  {"LIFX Downlight White to Warm", 1, 39, ADJ_WHITE, false, SINGLE, false},
  {"LIFX Downlight", 1, 40, COLOR, false, SINGLE, false},
  {"LIFX A19", 1, 43, COLOR, false, SINGLE, false},
  {"LIFX BR30", 1, 44, COLOR, false, SINGLE, false},
  {"LIFX A19 Night Vision", 1, 45, COLOR, true, SINGLE, false},
  {"LIFX BR30 Night Vision", 1, 46, COLOR, true, SINGLE, false},
  {"LIFX Mini Color", 1, 49, COLOR, false, SINGLE, false},
  {"LIFX Mini White to Warm", 1, 50, ADJ_WHITE, false, SINGLE, false},
  {"LIFX Mini White", 1, 51, WHITE, false, SINGLE, false},
  {"LIFX GU10", 1, 52, COLOR, false, SINGLE, false},
  {"LIFX GU10", 1, 53, COLOR, false, SINGLE, false},
  {"LIFX Tile", 1, 55, COLOR, false, MATRIX_CHAIN, false},
  {"LIFX Candle", 1, 57, COLOR, false, MATRIX, false},
  {"LIFX Mini Color", 1, 59, COLOR, false, SINGLE, false},
  {"LIFX Mini White to Warm", 1, 60, ADJ_WHITE, false, SINGLE, false},
  {"LIFX Mini White", 1, 61, WHITE, false, SINGLE, false},
  {"LIFX A19", 1, 62, COLOR, false, SINGLE, false},
  {"LIFX BR30", 1, 63, COLOR, false, SINGLE, false},
  {"LIFX A19 Night Vision", 1, 64, COLOR, true, SINGLE, false},
  {"LIFX BR30 Night Vision", 1, 65, COLOR, true, SINGLE, false},
  {"LIFX Mini White", 1, 66, WHITE, false, SINGLE, false},
  {"LIFX Candle", 1, 68, COLOR, false, MATRIX, false},
  {"LIFX Candle White to Warm", 1, 81, ADJ_WHITE, false, MATRIX, false},
  {"LIFX Filament Clear", 1, 82, WHITE, false, SINGLE, false},
  {"LIFX Filament Amber", 1, 85, WHITE, false, SINGLE, false},
  {"LIFX Mini White", 1, 88, WHITE, false, SINGLE, false},
  {"LIFX Clean", 1, 90, COLOR, false, SINGLE, true},
  {"LIFX Color", 1, 91, COLOR, false, SINGLE, false},
  {"LIFX Color", 1, 92, COLOR, false, SINGLE, false},
  {"LIFX A19 US", 1, 93, COLOR, false, SINGLE, false},
  {"LIFX BR30", 1, 94, COLOR, false, SINGLE, false},
  {"LIFX Candle White to Warm", 1, 96, ADJ_WHITE, false, SINGLE, false},
  {"LIFX A19", 1, 97, COLOR, false, SINGLE, false},
  {"LIFX BR30", 1, 98, COLOR, false, SINGLE, false},
  {"LIFX Clean", 1, 99, COLOR, false, SINGLE, true},
  {"LIFX Filament Clear", 1, 100, WHITE, false, SINGLE, false},
  {"LIFX Filament Amber", 1, 101, WHITE, false, SINGLE, false},
  {"LIFX A19 Night Vision", 1, 109, COLOR, true, SINGLE, false},
  {"LIFX BR30 Night Vision", 1, 110, COLOR, true, SINGLE, false},
  {"LIFX A19 Night Vision", 1, 111, COLOR, true, SINGLE, false},
  {"LIFX BR30 Night Vision", 1, 112, COLOR, true, SINGLE, false},
  {"LIFX Mini WW US", 1, 113, ADJ_WHITE, false, MATRIX, false},
  {"LIFX Mini WW Intl", 1, 114, ADJ_WHITE, false, MATRIX, false},
  {"", 1, 0, COLOR, false, SINGLE, false}
};


/*
 * Utility function to find the index of a particular PID.  Returns -1 if the PID can't
 * be found.
 */
int lifx_find_pid_index(uint32_t pid) {
	int i = 0;
	int p = -1;
	
	while (p != 0) {
		p = lifx_types[i].pid;
		if (p == pid) {
			return i;
		}
		i += 1;
	}
	
	return -1;
}

}
