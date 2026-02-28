/*
 * SOCD (Simultaneous Opposing Cardinal Directions) cleaning.
 *
 * Pure C module, no RTOS dependencies.
 *
 * Resolution strategy: last-input-wins.  When two opposing keys are
 * both active, the most recently pressed key takes priority.
 */

#ifndef SOCD_H
#define SOCD_H

#include <stdbool.h>
#include <stdint.h>

/* ---------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------ */

/**
 * Which key in an opposing pair was pressed most recently.
 */
enum socd_last {
    SOCD_LAST_NONE = 0,
    SOCD_LAST_A = 1,
    SOCD_LAST_B = 2,
};

/**
 * State for a single pair of opposing keys (e.g. left/right or up/down).
 *
 * @a_active     Whether key A is currently pressed.
 * @b_active     Whether key B is currently pressed.
 * @last_pressed Which key was pressed most recently.
 */
struct socd_pair {
    bool a_active;
    bool b_active;
    enum socd_last last_pressed;
};

/**
 * Resolved output for a pair.
 */
struct socd_result {
    bool a_output;
    bool b_output;
};

/* ---------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------ */

/**
 * Initialise a pair to the neutral (neither pressed) state.
 */
void socd_init(struct socd_pair *pair);

/**
 * Update the pair state when key A is pressed.
 */
void socd_press_a(struct socd_pair *pair);

/**
 * Update the pair state when key A is released.
 */
void socd_release_a(struct socd_pair *pair);

/**
 * Update the pair state when key B is pressed.
 */
void socd_press_b(struct socd_pair *pair);

/**
 * Update the pair state when key B is released.
 */
void socd_release_b(struct socd_pair *pair);

/**
 * Resolve the current state using last-input-wins.
 *
 * If both keys are active, only the most recently pressed one outputs
 * true.  If only one key is active, it outputs true.  If neither is
 * active, both output false.
 */
struct socd_result socd_resolve(const struct socd_pair *pair);

#endif /* SOCD_H */
