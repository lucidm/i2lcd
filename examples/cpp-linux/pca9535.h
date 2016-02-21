#ifndef __PCA9535_H__
#define __PCA9535_H__

#include <cstdint>
#include <string>
#include <exception>

using namespace std;

namespace i2lcd {

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
    CPORT,
    DPORT,
};

/**
 * @class PEXOpen
 *
 * @ingroup i2lcd
 *
 * @brief PEXOpen Exception class thrown when opening I2C device for
 *        PCA9535 will fail
 *
 *
 */
class PEXOpen: public exception
{
} tPEXOpen;

/**
 * @class PEXIOctl
 *
 * @ingroup i2lcd
 *
 * @brief PEXIOctl Exception class thrown when calling ioctl on I2C device for
 *        PCA9535 will fail
 *
 *
 */
class PEXIOctl: public exception
{
} tPEXIOctl;

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
	int fileh;
	uint8_t bus;
	uint8_t address;
	void _setRegister(t_PCARegs port, uint8_t value);
	uint8_t _getRegister(t_PCARegs port) const;
	uint8_t regval;

    public:
	PCA9535(uint8_t busn, uint8_t addressn);
	~PCA9535();
	void testPCA9535();


	uint8_t getDirection(t_PCAPort port) const;
	void setDirection(t_PCAPort port, uint8_t direction);

	uint8_t getPort(t_PCAPort port) const;

	void setOutput(t_PCAPort, uint8_t value);
	uint8_t getOutput(t_PCAPort port) const;

	uint8_t getPolarity(t_PCAPort port) const;
	void setPolarity(t_PCAPort, uint8_t value);

	uint8_t operator[](t_PCAPort port) const;
};

};
#endif
