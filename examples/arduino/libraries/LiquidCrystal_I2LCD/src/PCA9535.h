#ifndef __PCA9535_H__
#define __PCA9535_H__
#include <inttypes.h>

enum t_PCARegs {
    INPUT0 = 0,
    INPUT1,
    OUTPUT0,
    OUTPUT1,
    POLARITY0,
    POLARITY1,
    CONFIG0,
    CONFIG1,
};

enum t_PCAPort {
    CPORT = 0,
    DPORT,
};

/**
 * @class PCA9535
 *
 * @ingroup i2lcd
 *
 * @brief PCA9535 chip interface class
 *
 * Class implements methods for accessing PCA9535 registers
 *
 *
 */
class PCA9535
{
    private:
	int initialised;
	uint8_t bus;
	uint8_t address;
	int _setRegister(t_PCARegs port, uint8_t value);
	uint8_t _getRegister(t_PCARegs port) const;
	uint8_t regval;

        void test(void);

    public:
        int begin (uint8_t i2cAddr);

	uint8_t getDirection(t_PCAPort port) const;
	void setDirection(t_PCAPort port, uint8_t direction);

	uint8_t getPort(t_PCAPort port) const;

	void setOutput(t_PCAPort, uint8_t value);
	uint8_t getOutput(t_PCAPort port) const;

	uint8_t getPolarity(t_PCAPort port) const;
	void setPolarity(t_PCAPort, uint8_t value);

};

#endif
