/**
 * @file math/macro.h
 *
 * @brief Useful math-related macros
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __MATH_MACRO_H
#define __MATH_MACRO_H

/**
 * @brief Clamp a value between a minimum and maximum value
 *
 * @param[in] x   Value to clamp
 * @param[in] min Minimum value
 * @param[in] max Maximum value
 *
 * @return Clamped value
 */
inline float clamp(float x, float min, float max) {
    return fmaxf(fminf(x, max), min);
}

/**
 * @brief Clamp a value between 0 and 1
 *
 * @param[in] x Value to clamp
 *
 * @return Clamped value
 */
inline float saturate(float x) {
    return fmaxf(fminf(x, 1.0f), 0.0f);
}

#endif
