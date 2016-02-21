#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <pca9535.h>

int8_t openPCA9535(t_Pca9535 *pca, uint8_t bus, uint8_t address)
{
    char fname[16];

    pca->status = 0;
    snprintf(fname, 16, "/dev/i2c-%d", bus);
    pca->fileh = open(fname, O_RDWR);
    if (pca->fileh == -1)
    {
	fprintf(stderr, "Can't open %s device!\n", fname);
        exit(1);
    }
    else
        if (ioctl(pca->fileh, I2C_SLAVE, address) < 0)
        {
            fprintf(stderr, "Can't set slave address %d device!\n", address);
            exit(1);
        }



    pca->bus = bus;
    pca->address = address;

    return pca->status;
}

void closePCA9535(const t_Pca9535 *iface)
{
    if (iface)
	if (iface->fileh > 0)
	    close(iface->fileh);
}

void testPCA9535(t_Pca9535 *iface)
{
    uint8_t i, j;

    setPortOutput(iface, 0, 0x00);
    setPortOutput(iface, 1, 0x00);
    setPortDir(iface, 0, 0x00);
    setPortDir(iface, 1, 0x00);

    for(j = 0; j < 2; j++)
    {
	for(i = 0; i < 8; i++)
	{
	    setPortOutput(iface, j, 1 << i);
	    printf("P%d=%d\n", j, getPortOutput(iface, j));
	    sleep(1);
	}
	setPortOutput(iface, j, 0xFF);
	sleep(2);
	setPortOutput(iface, j, 0x00);
	setPortDir(iface, j, 0xFF);
    }
}

inline int8_t setPort(t_Pca9535 *iface, uint8_t port, uint8_t value)
{
    iface->status = i2c_smbus_write_byte_data(iface->fileh, port, value);
    return iface->status;
}

uint8_t getPort(t_Pca9535 *iface, uint8_t port)
{
    iface->status = 0;
    int8_t ret = i2c_smbus_read_byte_data(iface->fileh, port);
    if (errno < 0)
        iface->status = -1;
    return ret;
}

void setPortDir(t_Pca9535 *iface, uint8_t port, uint8_t direction)
{
    setPort(iface, CONF_PORT0 + port, direction);
}
uint8_t getPortDir(t_Pca9535 *iface, uint8_t port)
{
    return getPort(iface, CONF_PORT0 + port);
}

inline void setPortOutput(t_Pca9535 *iface, uint8_t port, uint8_t value)
{
    setPort(iface, OUTPUT_PORT0 + port, value);
}

uint8_t getPortOutput(t_Pca9535 *iface, uint8_t port)
{
    return getPort(iface, OUTPUT_PORT0 + port);
}

void setPortPolarity(t_Pca9535 *iface, uint8_t port, uint8_t value)
{
    setPort(iface, POLINV0 + port, value);
}

uint8_t getPortPolarity(t_Pca9535 *iface, uint8_t port)
{
    return getPort(iface, POLINV0 + port);
}

uint8_t getPortInput(t_Pca9535 *iface, uint8_t port)
{
    return getPort(iface, INPUT_PORT0 + port);
}
