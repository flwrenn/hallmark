/*
 * SOCD cleaning implementation -- last-input-wins.
 */

#include "socd.h"

/* --------------------------------------------------------------------- */

void socd_init(struct socd_pair *pair)
{
    pair->a_active = false;
    pair->b_active = false;
    pair->last_pressed = 0;
}

void socd_press_a(struct socd_pair *pair)
{
    pair->a_active = true;
    pair->last_pressed = 1;
}

void socd_release_a(struct socd_pair *pair)
{
    pair->a_active = false;
    if (pair->last_pressed == 1) {
        pair->last_pressed = pair->b_active ? 2 : 0;
    }
}

void socd_press_b(struct socd_pair *pair)
{
    pair->b_active = true;
    pair->last_pressed = 2;
}

void socd_release_b(struct socd_pair *pair)
{
    pair->b_active = false;
    if (pair->last_pressed == 2) {
        pair->last_pressed = pair->a_active ? 1 : 0;
    }
}

struct socd_result socd_resolve(const struct socd_pair *pair)
{
    struct socd_result r = {false, false};

    if (pair->a_active && pair->b_active) {
        /* Both active -- last input wins. */
        if (pair->last_pressed == 1) {
            r.a_output = true;
        } else {
            r.b_output = true;
        }
    } else {
        r.a_output = pair->a_active;
        r.b_output = pair->b_active;
    }

    return r;
}
