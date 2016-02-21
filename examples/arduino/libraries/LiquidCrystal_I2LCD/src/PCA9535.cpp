#include <Arduino.h>
#include <Wire.h>
#include "PCA9535.h"

int PCA9535::begin (uint8_t i2cAddr)
{
    address = i2cAddr;

    Wire.begin();
    Wire.beginTransmission ( address );
    Wire.write(0);
    Wire.read();
    initialised = (Wire.endTransmission() == 0);

    return initialised;
}

/**
 * @brief Helper method for setting the register of the chip
 * @param register number
 * @param value to set
 **/
int PCA9535::_setRegister(t_PCARegs port, uint8_t value)
{
   int status = 0;
   uint8_t sb[2];

   if ( initialised )
   {
      sb[0] = (uint8_t) port;
      sb[1] = value;
      Wire.beginTransmission ( address );
      Wire.write (sb, 2);
      status = Wire.endTransmission ();
   }
   return ( (status == 0) );

}

/**
 * @brief Helper method for getting value from register of the chip
 * @param register number
 * @return value of given register
 **/
uint8_t PCA9535::_getRegister(t_PCARegs port) const
{
    uint8_t retVal = 0;
    int status;

    if (initialised)
    {
        Wire.beginTransmission ( address );
        Wire.write ((uint8_t*)&port, 1);
        status = Wire.endTransmission ();

        Wire.requestFrom ( (uint8_t) address, (uint8_t) 1);
        retVal = Wire.read();

    }
    return retVal;
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


void PCA9535::test(void)
{
    uint8_t i, j;

    setOutput((t_PCAPort)0, 0x00);
    setOutput((t_PCAPort)1, 0x00);
    setDirection((t_PCAPort)0, 0x00);
    setDirection((t_PCAPort)1, 0x00);

    for(j = 0; j < 2; j++)
    {
	for(i = 0; i < 8; i++)
	{
	    setOutput((t_PCAPort)j, 1 << i);
            Serial.print("P");
            Serial.print(j);
            Serial.print("=");
            Serial.println(getOutput((t_PCAPort)j));
	    delay(1000);
	}
	setOutput((t_PCAPort)j, 0xFF);
	delay(2000);
	setOutput((t_PCAPort)j, 0x00);
	setDirection((t_PCAPort)j, 0xFF);
    }
}
