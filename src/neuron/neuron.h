#ifndef NEURON_H
#define NEURON_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../raylib/raylib.h"

#include "../misc/types.h"

#define NEURON_COUNT (1ull << 14)

#define SENSOR_NEURON_COUNT (1ull << 12) // all excitatory
#define PACEMAKER_NEURON_COUNT (1ull << 8) // all excitatory

#define BASIC_NEURON_COUNT (NEURON_COUNT - SENSOR_NEURON_COUNT - PACEMAKER_NEURON_COUNT)

#define INHIBITOR_NEURON_COUNT (BASIC_NEURON_COUNT >> 2)
#define EXCITOR_NEURON_COUNT (BASIC_NEURON_COUNT - INHIBITOR_NEURON_COUNT)

#define NEURON_TARGET_COUNT (1ull << 6)


#define EXCITOR_START 0
#define EXCITOR_END (EXCITOR_NEURON_COUNT)

#define INHIBITOR_START (EXCITOR_END)
#define INHIBITOR_END (INHIBITOR_START + INHIBITOR_NEURON_COUNT)

#define PACEMAKER_START (INHIBITOR_END)
#define PACEMAKER_END (PACEMAKER_START + PACEMAKER_NEURON_COUNT)

#define SENSOR_START (PACEMAKER_END)
#define SENSOR_END (SENSOR_START + SENSOR_NEURON_COUNT)

static_assert(SENSOR_END == NEURON_COUNT, "neuron ranges should add up to the neuron count");

// whether a neuron is a sensor, pacemaker, excitor, or inhibitor is determined by its position in the array

typedef struct neuron_t {
    uchar charge; // when this reaches 255, the neuron gains a pulse

    uchar pulse_counter; // when this is >0, the neuron pulses until it reaches 0

    uchar pulse_timer; // counts down, when it reaches input_strength, the neuron pulses, adding +-2 charge to each target

    uchar input_strength; // 0 means no sensory input, timer counts down to this, then pulses

    uchar pulse_freshness;

    uint32 targets[NEURON_TARGET_COUNT]; // indeces of the targets

} neuron_t;


neuron_t* new_neuron_array();

void neuron_tick(neuron_t* neurons);


#ifdef IMPL_

uint32_t g_neuron_index = 0;


neuron_t* new_neuron_array() {
    neuron_t* neurons = malloc(NEURON_COUNT * sizeof(neuron_t));
    assert(neurons);
    // intentionally leaves memory uninitialized for the randomness and chaos

    // randomize memory incase malloc returns zeroed out memory
    srand(time(NULL));
    uchar* bytes = (uchar*)neurons;
    size_t total_bytes = NEURON_COUNT * sizeof(neuron_t);
    for (size_t i = 0; i < total_bytes; i++) {
        bytes[i] = rand() & 0xFF;
    }

    // mask out top bits so that all target indices are basic neurons
    for (neuron_t *it = neurons, *end = neurons + NEURON_COUNT; it < end; it++) {
        for (uint32 *it2 = it->targets, *end2 = it->targets + NEURON_TARGET_COUNT; it2 < end2; it2++) {
            //printf("%u ", *it2);
            *it2 %= BASIC_NEURON_COUNT;
            //printf("%u ", *it2);
        }
    }

    return neurons;
}

uchar saturated_add_uchar(uchar a, uchar b) {
    uint16_t sum = (uint16_t)a + (uint16_t)b;

    return (uint8_t)(sum > UINT8_MAX ? UINT8_MAX : sum);
}

uchar saturated_sub_uchar(uchar a, uchar b) {
    uchar diff = a - b;

    return (uchar)(a < b ? 0 : diff);
}

void fire_excitor(neuron_t* neuron, neuron_t* neurons) {
    for (uint32 *it = neuron->targets, *end = neuron->targets + NEURON_TARGET_COUNT; it < end; it++) {
        neurons[*it].charge = saturated_add_uchar(neurons[*it].charge, 2);
        if (neurons[*it].charge == 255) {
            neurons[*it].pulse_counter = saturated_add_uchar(neurons[*it].pulse_counter, 1);
            neurons[*it].charge = 0;
        }
    }
    neuron->pulse_freshness = 255;
    //printf("%u ", g_neuron_index);
}

void fire_inhibitor(neuron_t* neuron, neuron_t* neurons) {
    for (uint32 *it = neuron->targets, *end = neuron->targets + NEURON_TARGET_COUNT; it < end; it++) {
        neurons[*it].charge = saturated_sub_uchar(neurons[*it].charge, 1);
    }
    neuron->pulse_freshness = 255;
    //printf("%u ", g_neuron_index);
}

void try_fire_pacemaker(neuron_t* neuron, neuron_t* neurons) {
    if (neuron->pulse_timer <= neuron->input_strength) {
        fire_excitor(neuron, neurons);
        neuron->pulse_counter = saturated_sub_uchar(neuron->pulse_counter, 1);
        neuron->pulse_timer = 255;
    }
}

void try_fire_sensor(neuron_t* neuron, neuron_t* neurons) {
    if (neuron->input_strength > 0 && neuron->pulse_timer <= neuron->input_strength) {
        fire_excitor(neuron, neurons);
        neuron->pulse_timer = 255;
    }
}

void try_fire_excitor(neuron_t* neuron, neuron_t* neurons) {
    if (neuron->pulse_counter > 0 && neuron->pulse_timer <= neuron->input_strength) {
        fire_excitor(neuron, neurons);
        neuron->pulse_counter = saturated_sub_uchar(neuron->pulse_counter, 1);
        neuron->pulse_timer = 255;
    }
}

void try_fire_inhibitor(neuron_t* neuron, neuron_t* neurons) {
    if (neuron->pulse_counter > 0 && neuron->pulse_timer <= neuron->input_strength) {
        fire_inhibitor(neuron, neurons);
        neuron->pulse_counter = saturated_sub_uchar(neuron->pulse_counter, 1);
        neuron->pulse_timer = 255;
    }
}

void process_neuron_timer(neuron_t* neuron) {
    neuron->pulse_timer = saturated_sub_uchar(neuron->pulse_timer, 1);
}

void process_senses(neuron_t* neuron, uint32 neuron_index) {
    // empty for now, will probably take an image or something as input
}



void neuron_tick(neuron_t* neurons) {
    g_neuron_index = 0;
    
    int pos_x = -1;
    int pos_y = 0;

    void* current_neuron_type_label = &&EXCITOR_LABEL; // g_neuron_index love computed goto



    for (neuron_t *it = neurons, *end = neurons + NEURON_COUNT; it < end; it++, g_neuron_index++) {

        pos_x++;

        if (pos_x == 128) {
            pos_y++;
            if (pos_y == 128) {
                pos_y = 0;
            }
            pos_x = 0;
        }


        DrawPixel(pos_x, pos_y, (Color){.a = 255, .r = it->pulse_freshness, .b = it->charge, .g = it->pulse_timer});

        it->pulse_freshness = saturated_sub_uchar(it->pulse_freshness, 8);

        process_neuron_timer(it);

        // charge decay
        it->charge = saturated_sub_uchar(it->charge, 1);

        goto *current_neuron_type_label;

        EXCITOR_LABEL: {

            try_fire_excitor(it, neurons);

            if (g_neuron_index+1 >= EXCITOR_END) {
                current_neuron_type_label = &&INHIBITOR_LABEL;
            }
            continue;
        }
        INHIBITOR_LABEL: {

            try_fire_inhibitor(it, neurons);

            if (g_neuron_index+1 >= INHIBITOR_END) {
                current_neuron_type_label = &&PACEMAKER_LABEL;
            }
            continue;
        }
        PACEMAKER_LABEL: {

            try_fire_pacemaker(it, neurons);

            if (g_neuron_index+1 >= PACEMAKER_END) {
                current_neuron_type_label = &&SENSOR_LABEL;
            }
            continue;
        }
        SENSOR_LABEL: {

            it->input_strength = saturated_sub_uchar(it->input_strength, 1);

            process_senses(it, g_neuron_index);

            try_fire_sensor(it, neurons);

            continue;
        }


    }
}

#endif
#endif