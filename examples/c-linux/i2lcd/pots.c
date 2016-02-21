#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <i2lcd.h>
#include <pots.h>

inline int nsleep(long nanos)
{
    struct timespec ts, rm;
    ts.tv_sec = 0;
    ts.tv_nsec = nanos;

    return nanosleep(&ts, &rm);
}

void openPotentiometer(t_Pca9535 *iface, t_Potentiometer *pot, uint8_t cs, uint8_t ud)
{
    uint8_t d;

    pot->iface = iface;
    pot->csb = cs;
    pot->udb = ud;
    d = getPortDir(pot->iface, CPORT);
    setPortDir(pot->iface, CPORT, (d & (~(cs | ud))));

    pot->control = getPortOutput(pot->iface, CPORT);
    setPortOutput(pot->iface, CPORT, pot->control | cs | ud);
    pot->current = 0x00;

    for(d=0; d < 64; d++)
	decPot(pot);
}


void closePotentiometer(const t_Potentiometer *pot)
{
    uint8_t d;
    if (pot)
    {
	d = getPortDir(pot->iface, CPORT);
	setPortDir(pot->iface, CPORT, d | pot->csb | pot->udb);
    }
}


void csPot(t_Potentiometer *pot, uint8_t value)
{
    pot->control = value ? (pot->control | pot->csb) : (pot->control & (~pot->csb));
    setPortOutput(pot->iface, CPORT, pot->control);
}

void udPot(t_Potentiometer *pot, uint8_t value)
{
    pot->control = value ? (pot->control | pot->udb) : (pot->control & (~pot->udb));
    setPortOutput(pot->iface, CPORT, pot->control);
}

void incPot(t_Potentiometer *pot)
{
    pot->current += pot->current < 0x40 ? 1 : 0;
    udPot(pot, 1);
    nsleep(750);
    csPot(pot, 0);
    udPot(pot, 0);
    nsleep(500);
    udPot(pot, 1);
    usleep(5);
    csPot(pot, 1);
}

void decPot(t_Potentiometer *pot)
{
    pot->current -= pot->current > 0 ? 1 : 0;
    udPot(pot, 0);
    nsleep(750);
    csPot(pot, 0);
    udPot(pot, 1);
    nsleep(500);
    udPot(pot, 0);
    usleep(5);
    csPot(pot, 1);
    usleep(5);
}

void setPot(t_Potentiometer *pot, uint8_t value)
{
    uint8_t diff, i;

    value %= 0x40;
    if (value > pot->current)
    {
	diff = value - pot->current;
	udPot(pot, 1);
	nsleep(750);
	csPot(pot, 0);
	for (i=0; i < diff; i++)
	{
	    udPot(pot, 0);
	    nsleep(500);
	    udPot(pot, 1);
	    nsleep(500);
	}
	usleep(5);
	csPot(pot,1);
	if ((pot->current + diff) > 0x3f)
	    pot->current = 0x3f;
	else
	    pot->current += diff;
    } else if (value < pot->current)
    {
	diff = pot->current - value;
	udPot(pot, 0);
	nsleep(750);
	csPot(pot, 0);
	for (i=0; i < diff; i++)
	{
	    udPot(pot, 1);
	    nsleep(500);
	    udPot(pot, 0);
	    nsleep(500);
	}
	usleep(5);
	csPot(pot, 1);
	if ((pot->current - diff) < 0)
	    pot->current = 0;
	else
	    pot->current -= diff;
    }

}
