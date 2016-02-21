#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include <unistd.h>

#include <iostream>
#include <string>
#include <cstdio>

#include <pca9535.h>
using namespace i2lcd;

/**
 * @brief Class constructor
 * @param bus number
 * @param chip address
 **/
PCA9535::PCA9535(uint8_t busn, uint8_t addressn) : bus(busn), address(addressn)
{
    string s = "/dev/i2c-" + to_string(bus);

    fileh = open(s.c_str(), O_RDWR);
    if (fileh == -1)
	throw tPEXOpen;

    if (ioctl(fileh, I2C_SLAVE, address) < 0)
	throw tPEXIOctl;
}

/**
 * @brief Class destructor
 **/
PCA9535::~PCA9535()
{
    if (fileh)
	close(fileh);
}


/**
 * @brief Private method hiding the fact we're
 *        using linux file to access I2C bus
 * @param register number
 * @param value to set
 **/
void PCA9535::_setRegister(t_PCARegs port, uint8_t value)
{
    i2c_smbus_write_byte_data(fileh, port, value);
}

/**
 * @brief Private method hiding the fact we're
 *        using linux file to access I2C bus
 * @param register number
 * @return value of given register
 **/
uint8_t PCA9535::_getRegister(t_PCARegs port) const
{
    return i2c_smbus_read_byte_data(fileh, port);
}

/**
 * @brief Return current direction bit mask of given port
 * @param port number (0 or 1)
 * @return byte with 1 if direction of matching bit of given
 *         port is set to input or 0 for output
 **/
uint8_t PCA9535::getDirection(t_PCAPort port) const
{
    return _getRegister((t_PCARegs) (CONFIG0 + port));
}

/**
 * @brief Set direction of given port
 * @param port number
 * @param byte with 0s set for bits set to work as output
 *        1s for port lines working as input
 **/
void PCA9535::setDirection(t_PCAPort port, uint8_t direction)
{
    _setRegister((t_PCARegs) (CONFIG0 + port), direction);
}

/**
 * @brief Get value of all bits set as input
 * @param port number
 * @return all input working lines of the port will report
 *         its current state in return value
 **/
uint8_t PCA9535::getPort(t_PCAPort port) const
{
    return _getRegister((t_PCARegs) (INPUT0 + port));
}

/**
 * @brief Set value of all bits set as output
 * @param port number
 * @param all lines of given port will working as output
 *         will be set according to bits in value
 **/
void PCA9535::setOutput(t_PCAPort port, uint8_t value)
{
    _setRegister((t_PCARegs) (OUTPUT0 + port), value);
}

/**
 * @brief Return current output register
 * @param port number
 * @return current set value of output port register
 **/
uint8_t PCA9535::getOutput(t_PCAPort port) const
{
    return _getRegister((t_PCARegs) (OUTPUT0 + port));
}

/**
 * @brief Return current output polarity register
 *        the PCA9535 chip has functionality to reverse
 *        logic of port bits. Writing 0 gives 1 on matching
 *        lines and 1s gives 0s. 1s on bit values returned from this
 *        function means, the logic is reversed.
 * @param port number
 * @return current set value of polarity port register
 **/
uint8_t PCA9535::getPolarity(t_PCAPort port) const
{
    return _getRegister((t_PCARegs) (POLARITY0 + port));
}

/**
 * @brief Set current output polarity register
 *        the PCA9535 chip has functionality to reverse
 *        logic of port bits. Writing 0 gives 1 on matching
 *        lines and 1s gives 0s. 1s on bit values returned from this
 *        function means, the logic is reversed.
 * @param port number
 * @param value byte with all bits set to 1 will reverse polarity of
 *        given port.
 **/
void PCA9535::setPolarity(t_PCAPort port, uint8_t value)
{
    _setRegister((t_PCARegs) (POLARITY0 + port), value);
}

/**
 * @brief Class accessor for input registers as [] operator
 *        method. Index 0 for port 0 and 1 for port #1.
 * @return input value of given port number.
 **/
uint8_t PCA9535::operator[](t_PCAPort port) const
{
    return _getRegister((t_PCARegs) (INPUT0 + port));
}

