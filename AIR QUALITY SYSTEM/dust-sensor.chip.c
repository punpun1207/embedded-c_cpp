// Wokwi Custom Chip - For docs and examples see:
// https://docs.wokwi.com/chips-api/getting-started
//
// SPDX-License-Identifier: MIT
// Copyright 2023 Muhamad Kassim

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
} chip_state_t;

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  printf("Custom chip");
}
