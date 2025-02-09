#ifndef DESENHO_H
#define DESENHO_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

extern int digitos[][25];

void desenho_pio(int desenho[25], PIO pio, uint sm);

#endif