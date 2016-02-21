
#include <unistd.h>
#include <time.h>
#include <pots.h>

using namespace i2lcd;

/**
 * @brief Helper function to wait in ns time spans. In average CPUs it is not
 * really necessary, but on faster CPUs no using such delay, may cause
 * communication problems with the chips.
 */
inline int nsleep(long nanos)
{
    struct timespec ts, rm;
    ts.tv_sec = 0;
    ts.tv_nsec = nanos;
    return nanosleep(&ts, &rm);
}

/**
 * @brief Class constructor
 * @param PCA9535 chip class for communication with pots.
 * @param cs - line number where CS line of MCP401x is connected to
 * @param ud - line number where UD line of MCP401x is connected to
 **/
Potentiometer::Potentiometer(PCA9535 &chip, t_PotBits cs, t_PotBits ud) : current(0), csb((uint8_t) cs), udb((uint8_t) ud), iface(chip)
{
    uint8_t tmp;

    tmp = iface.getDirection(CPORT);
    iface.setDirection(CPORT, tmp & (~(csb | udb)));

    control = iface.getOutput(CPORT);
    iface.setOutput(CPORT, control | csb | udb);

    for(tmp=0; tmp < 64; tmp++)
	dec();


}

/**
 * @brief Class destructor
 **/
Potentiometer::~Potentiometer()
{
    uint8_t tmp;

    set(0x00);
    tmp = iface.getDirection(CPORT);
    iface.setDirection(CPORT, tmp | csb | udb);
}

/**
 * @brief Set state of CS line
 * @param True for 1, False for 0
 **/
void Potentiometer::cs(bool value)
{
    control = value ? (control | csb) : (control & (~csb));
    iface.setOutput(CPORT, control);
}

/**
 * @brief Set state of UD line
 * @param True for 1, False for 0
 **/
void Potentiometer::ud(bool value)
{
    control = value ? (control | udb) : (control & (~udb));
    iface.setOutput(CPORT, control);
}

/**
 * @brief Decrement current value of potentiometer by 1
 **/
void Potentiometer::dec()
{
    current -= current > 0 ? 1 : 0;
    ud(0);
    nsleep(750);
    cs(0);
    ud(1);
    nsleep(500);
    ud(0);
    usleep(5);
    cs(1);
    usleep(5);
}

/**
 * @brief Increment current value of potentiometer by 1
 **/
void Potentiometer::inc()
{
    current += current < 63 ? 1 : 0;
    ud(1);
    nsleep(750);
    cs(0);
    ud(0);
    nsleep(500);
    ud(1);
    usleep(5);
    cs(1);
    usleep(5);
}

/**
 * @brief Set potentiometer to given value.
 *        Will set value of pot by calculating delta between
 *        requested value and current value and depend of sign of the delta
 *        will decrement/increment pot by one until desired value is reached.
 *        These potentiometers don't have any register we can read to get current
 *        wiper position, so we must remeber state of pot in class variable.
 * @param value
 **/
void Potentiometer::set(uint8_t value)
{
    uint8_t diff, i;

    value %= 0x40;

    if (value > current)
    {
	diff = value - current;
	ud(1);
	nsleep(750);
	cs(0);
	for (i=0; i < diff; i++)
	{
	    ud(0);
	    nsleep(500);
	    ud(1);
	    nsleep(500);
	}
	usleep(5);
	cs(1);
	if ((current + diff) > 0x3f)
	    current = 0x3f;
	else
	    current += diff;
    } else if (value < current)
    {
	diff = current - value;
	ud(0);
	nsleep(750);
	cs(0);
	for(i=0; i < diff; i++)
	{
	    ud(1);
	    nsleep(500);
	    ud(0);
	    nsleep(500);
	}
	usleep(5);
	cs(1);
	if ((current - diff) < 0)
	    current = 0;
	else
	    current -= diff;
    }
}
