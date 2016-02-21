#ifndef __POTS_H__
#define __POTS_H__

#include <cstdint>
#include <pca9535.h>

using namespace i2lcd;

namespace i2lcd {

enum t_PotBits {
    UD = 1 << 5,
    CONTRAST_CS = 1 << 6,
    BACKLIGHT_CS = 1 << 7,
};

/**
 * @class Potentiometer
 *
 * @ingroup i2lcd
 *
 * @brief Class for controlling potentiometers in I2LCD module.
 *        Same for backlight intensity and contrast.
 *        Module is using MCP401x chips, so this class is realization
 *        of it's up/down protocol.
 *
 */
class Potentiometer
{
    private:
	uint8_t control;
	uint8_t current;
	uint8_t csb;
	uint8_t udb;
	PCA9535 &iface;

	void dec();
	void inc();
	void ud(bool value);
	void cs(bool value);

    public:
	Potentiometer(PCA9535 &chip, t_PotBits csb, t_PotBits ud);
	~Potentiometer();

	void set(uint8_t value);
};



};


#endif
