#ifndef __PCA9535_H__
#define __PCA9535_H__
#include <stdint.h>

/**
 * @brief PCA9535 registers definition.
 * The chip contain 16 bidirectional lines divided between two ports. It has
 * 4 groups of two registers, one for each port. Ports controls direction of
 * each pin, you can read current state of input pins, set state of output pins
 * or invert logic of given pins.
 */
#define INPUT_PORT0	0 /**< Port 0 input value */
#define INPUT_PORT1	1 /**< Port 1 input value */
#define OUTPUT_PORT0	2 /**< Port 0 output value */
#define OUTPUT_PORT1	3 /**< Port 1 output value */
#define POLINV0		4 /**< Port 0 inversion logic register */
#define POLINV1		5 /**< Port 1 inversion logic register */
#define CONF_PORT0	6 /**< Direction control register for Port 0 */
#define CONF_PORT1	7 /**< Direction control register for Port 1 */


/**
 * @brief structure holds current state of the chip.
 */
typedef struct s_pca9535 {
    int fileh;  /**< Communication with chip is done by /dev/i2c-* device file this is file handler number */
    uint8_t bus; /**< Bus number the chip is connected to */
    uint8_t address; /**< I2C Address of the chip */
    int8_t status; /**< Status of communication with the chip, if -1, something went wrong */
} t_Pca9535;

/**
 * @brief Open device file for the chip connected to given bus at given address
 * @param *pca address of previously allocated structure to be filled with values
 * @param *bus I2C bus number
 * @param *address address of the chip
 */
int8_t openPCA9535(t_Pca9535 *pca, uint8_t bus, uint8_t address);

/**
 * @brief Close device file opened by openPCA9535 function
 * @param *pca address of previously allocated t_Pca9535 structure
 */
void closePCA9535(const t_Pca9535 *iface);

void testPCA9535(t_Pca9535 *iface);

/**
 * @brief This function will set register given as
 *        parameter to given value.
 * @param *iface address of t_Pca9535 structure
 * @param port register number as defined above
 * @param value value to wichi port will be set
 *
 * @return 0 if call was executed, otherwise -1
 */
int8_t setPort(t_Pca9535 *iface, uint8_t port, uint8_t value);

/**
 * @brief Function will return value of given register, read from the chip.
 * @param *iface address of t_Pca9535 structure
 * @param port register number which should be read.
 * @return register value
 */
uint8_t getPort(t_Pca9535 *iface, uint8_t port);

/**
 * @brief Set direction of given port. As PCA9535 has two ports, we can
 *        give them numbers like 0 and 1. This function excpects 0 for the first
 *        port of the chip and 1 for the second one.
 * @param *iface address of t_Pca9535 structure
 * @param port port number which direction should be set (0 - first one, 1 - second one)
 * @param value bits set to 1 in this byte will make appropriate lines of the port
 *              inputs. 0 will make those lines to act as output.
 */
void setPortDir(t_Pca9535 *iface, uint8_t port, uint8_t value);

/**
 * @brief Return current direction register value.
 * @param *iface address of t_Pca9535 structure
 * @param port port number
 * @return direction register value
 */
uint8_t getPortDir(t_Pca9535 *iface, uint8_t port);

/**
 * @brief Set output lines of the port to given value. Bits in given value for
 *        lines set as input will be ignored.
 * @param *iface address of the t_Pca9535 structure
 * @param port port number
 */
void setPortOutput(t_Pca9535 *iface, uint8_t port, uint8_t value);

/**
 * @brief Return current output register value for given port.
 * @param *iface address of the t_Pca9535 structure
 * @param port port number
 * @return output port register value
 */
uint8_t getPortOutput(t_Pca9535 *iface, uint8_t port);

/**
 * @brief Set polarity inversion of given port. All lines set for inverse logic will
 *        have reverse polarity, which means 1 write to output will cause the line
 *        to go to low state and 1 if 0 is written.
 * @param *iface address of the t_Pca9535 structure
 * @param port port number
 * @param value bit set 1 will reverse polarity of appropriate line.
 */
void setPortPolarity(t_Pca9535 *iface, uint8_t port, uint8_t value);

/**
 * @brief Get polarity inversion register value
 * @param *iface address of the t_Pca9535 structure
 * @param port port number
 * @return current value of inversion polarity register
 */
uint8_t getPortPolarity(t_Pca9535 *iface, uint8_t port);

/**
 * @brief Function return current state of lines of port set as input.
 *        Lines set as output will be ignored. You should mask those bits
 *        in tyour code.
 * @param *iface address of the t_Pca9535 structure
 * @param port port number
 * @return current input register value
 */
uint8_t getPortInput(t_Pca9535 *iface, uint8_t port);

#endif
