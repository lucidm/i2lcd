#ifndef __POTS_H__
#define __POTS_H__
#include <pca9535.h>

/**
 * @brief Potentiometers pins definitions
 *
 */
#define UD  (1 << 5) /**< Up/Down pin is common for both pots */
#define CCS (1 << 6) /**< Pin for CS signal of contrast pot. */
#define BCS (1 << 7) /**< Pin for CS singal of backlight pot */


/**
 * @brief Holds state of potentiometer
 *
 */
typedef struct s_potentiometer {
    uint8_t control; /**< State of control port bits */
    uint8_t current; /**< Current value of potentiometer counter */
    uint8_t csb;    /**< bit for CS line of potentiometer */
    uint8_t udb;    /**< bit for Up/Down line of potentiometer */
    t_Pca9535 *iface; /**< Address of t_PCA9535 struct for communication with pots */
} t_Potentiometer;


/**
 * @brief fill pot structure with proper values, set lines controlling pot
 *        to output and set potentiometer to default state.
 *
 * @param *iface address of previously allocated and initialized structure for PCA9535
 *               chip
 * @param *pot address of t_Potentiometer structure which will be filled with proper
 *             values
 * @param cs   bit value of cs line
 * @param ud   bit value of ud line
 */
void openPotentiometer(t_Pca9535 *iface, t_Potentiometer *pot, uint8_t cs, uint8_t ud);

/**
 * @brief close potentiometer ports. Will change them to inputs
 * @param *pot address of structure previously allocated and initialized by
 *             openPotentiometer(...) function.
 */
void closePotentiometer(const t_Potentiometer *pot);

/**
 * @brief Increment value of potentiometer. If value reach 0x3f it will be held
 *        at this position.
 * @param *pot address of structure holding potentiometer definiction.
 */
void incPot(t_Potentiometer *pot);

/**
 * @brief Decrement value of potentiometer. If value reach 0x00 this function will
 *        do nothing.
 * @param *pot potentiometer stucture address.
 *
 */
void decPot(t_Potentiometer *pot);

/**
 * @brief Set potentiometer to given value. This function will calculate current
 *        wiper position of pot and as we are unable to read current position from
 *        pot, will increase or decrease wiper position to reach given value.
 *        Maximum allowed value is 0x3f, minimum is 0x00.
 * @param *pot potentiometer stucture address.
 * @param value wiper value to set
 */
void setPot(t_Potentiometer *pot, uint8_t value);
#endif
