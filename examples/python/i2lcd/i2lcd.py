# -*- coding: utf-8 -*-

from time import sleep
import textwrap
import lcdconst
from pca9535 import PCA9535
from pots import Potentiometer
from statusflag import DelayWaiter, BusyLoop

def _checkPos(function):
    def _f(*args):
	obj = args[0]
	if args[1] >= obj.cols or args[1] < 0:
	    raise ValueError('Column {0} is not between 0-{0}.'.format(args[1], obj.cols - 1))

	if args[2] >= obj.rows or args[2] < 0:
	    raise ValueError('Row {0} is not between 0-{0}.'.format(args[2], obj.rows - 1))

	return function(*args)
    return _f



class I2LCD(object):
    memaddr = {	16 : {0: [0x00], 1 : [0x00, 0x40], 2 : [0x00, 0x40], 4 : [0x00, 0x40, 0x10, 0x50]},
	        20 : {2 : [0x00, 0x40], 4 : [0x00, 0x40, 0x14, 0x54]},
		40 : {2 : [0x00, 0x40], 4 : [0x00, 0x40, 0x00, 0x40]}
	      }


    def __init__(self, bus, address, cols, rows):
	self.iface = PCA9535(bus, address)
	

	if not self.memaddr.has_key(cols):
	    raise NotImplementedError
	if not self.memaddr[cols].has_key(rows):
	    raise NotImplementedError

	self.rowsaddress = self.memaddr[cols][rows]
	self.cols = 8 if rows == 1 else cols
	self.rows = 2 if rows == 1 else rows if rows else 1
	self.column = 0
	self.row = 0
	self.control = 0x00
	self.setControl(self.control, 0)
	self.iface.setPortDir(lcdconst.CPORT, lcdconst.IRS | lcdconst.UD | lcdconst.BCS | lcdconst.CCS)
	self.iface.setPortDir(lcdconst.DPORT, 0x00)
	self.wait = DelayWaiter(self)
	self.iface.setPortOutput(lcdconst.DPORT, 0x00)

	self.cpot = Potentiometer(self.iface, lcdconst.CCS, lcdconst.UD)
	self.bpot = Potentiometer(self.iface, lcdconst.BCS, lcdconst.UD)

	self.control |= self.iface.getPortOutput(lcdconst.CPORT)
	self.setControl(self.control, 1)
	self.commands = {lcdconst.CLEAR_DISPLAY : 0, 
			    lcdconst.CURSOR_HOME : 0, 
			    lcdconst.ENTRY_MODE_SET : 0, 
			    lcdconst.DISPLAY_ONOFF : 0, 
			    lcdconst.CURSOR_DISPLAY_SHIFT : 0, 
			    lcdconst.FUNCTION_SET : 0, 
			    lcdconst.SET_CGRAM_ADDRESS : 0, 
			    lcdconst.SET_DDRAM_ADDRESS : 0}
	self.waitflag = 0

    def close(self):
	self.setContrast(0x00)
	self.setBacklight(0x00)
	self.setControl(lcdconst.EN | lcdconst.PWR, 0)
	self.iface.setPortDir(lcdconst.CPORT, 0xFF)
	self.iface.setPortDir(lcdconst.DPORT, 0xFF)

    def setControl(self, control, value):
	self.control = (self.control | control) if value else (self.control & ~control)
	self.iface.setPortOutput(lcdconst.CPORT, self.control)

    def setDirection(self, direction):
	self.iface.setPortDir(lcdconst.DPORT, direction)

    def command(self, command, value):
	while (self.waitflag and self.readStatus()[0]):
	    continue
	data = command | value
	self.setControl(lcdconst.RS | lcdconst.RW, 0)
	self.setControl(lcdconst.EN, 1)
	self.iface.setPortOutput(lcdconst.DPORT, data)
	self.setControl(lcdconst.RS | lcdconst.RW | lcdconst.EN, 0)
	self.commands[command] = value

    def readStatus(self):
	self.setControl(lcdconst.RS, 0)
	self.setControl(lcdconst.RW, 1)
	self.setDirection(0xFF)
	self.setControl(lcdconst.EN, 1)
	ret = self.iface.getPortInput(lcdconst.DPORT)
	self.setControl(lcdconst.RS | lcdconst.RW | lcdconst.EN, 0)
	self.setDirection(0x00)
	return (ret & lcdconst.BF, ret & ~lcdconst.BF)

    def writeByte(self, value):
	self.setControl(lcdconst.RS, 1)
	self.setControl(lcdconst.RW, 0)
	self.setControl(lcdconst.EN, 1)
	self.iface.setPortOutput(lcdconst.DPORT, value)
	self.setControl(lcdconst.RS | lcdconst.RW | lcdconst.EN, 0)

    def readBlock(self, l):
	self.setControl(lcdconst.RS, 1)
	self.setControl(lcdconst.RW, 1)
	ret = []
	for b in xrange(l):
	    self.setControl(lcdconst.EN, 1)
	    ret.append(self.iface.getPortInput(lcdconst.DPORT))
	    self.setControl(lcdconst.EN, 0)
	self.setControl(lcdconst.RS | lcdconst.RW | lcdconst.EN, 0)
	return ret

    def writeBlock(self, values):
	self.setControl(lcdconst.RS, 1)
	self.setControl(lcdconst.RW, 0)
	for b in values:
	    self.setControl(lcdconst.EN, 1)
	    self.iface.setPortOutput(lcdconst.DPORT, b)
	    self.setControl(lcdconst.EN, 0)
	self.setControl(lcdconst.RS | lcdconst.RW | lcdconst.EN, 0)

    def readDataByte(self):
	self.setControl(lcdconst.RS, 1)
	self.setControl(lcdconst.RW, 1)
	self.setDirection(0xFF)
	self.setControl(lcdconst.EN, 1)
	ret = self.iface.getPortInput(lcdconst.DPORT)
	self.setControl(lcdconst.RS | lcdconst.RW | lcdconst.EN, 0)
	self.setDirection(0x00)
	return ret

    def readRow(self, row):
	self.command(lcdconst.SET_DDRAM_ADDRESS, self.rowsaddress[row])
	self.setControl(lcdconst.RS, 1)
	self.setControl(lcdconst.RW, 1)
	self.setDirection(0xFF)
	ret = ""
	for i in range(self.cols):
	    self.setControl(lcdconst.EN, 1)
	    ret += chr(self.iface.getPortInput(lcdconst.DPORT))
	    self.setControl(lcdconst.EN, 0)
	self.setControl(lcdconst.RS | lcdconst.RW, 0)
	self.setDirection(0x00)
	self.setCursor(self.column, self.row)
	return ret

    @_checkPos
    def setCursor(self, column, row):
	row %= self.rows
	column %= self.cols
	self.column = column
	self.row = row
	address = self.rowsaddress[row]+column
	self.command(lcdconst.SET_DDRAM_ADDRESS, address)

    def clear(self):
	self.command(lcdconst.CLEAR_DISPLAY, 0x00)
	self.column = 0
	self.row = 0

    def home(self):
	self.command(lcdconst.CURSOR_HOME, 0x00)
	self.column = 0
	self.row = 0

    def setGC(self, number, definition):
	self.command(lcdconst.SET_CGRAM_ADDRESS, ((number & 0x07) << 3))
	self.writeBlock(definition)

    def fprint(self, string):
	column = self.column
	row = self.row

	for i, line in enumerate(string.rstrip().split('\n')):
	    self.command(lcdconst.SET_DDRAM_ADDRESS, self.rowsaddress[row]+column)
	    wrap = textwrap.wrap(line, self.cols if column == 0 else self.cols - column)
	    for j, txt in enumerate(wrap):
		self.writeBlock([ord(c) for c in txt])
		column += len(txt)
		if column >= self.cols:
		    column = 0
		    row = (row + 1) % self.rows
		    self.command(lcdconst.SET_DDRAM_ADDRESS, self.rowsaddress[row]+column)
	    if i > 0:
		column = 0
		row = (row + 1) % self.rows
	self.column = column % self.cols
	self.row = row

    def init(self):
	if self.rows == 1:
	    fn = 0
	else:
	    fn = lcdconst.FS_N

	self.command(lcdconst.FUNCTION_SET, lcdconst.FS_DL | fn)
	sleep(0.045)
	self.command(lcdconst.FUNCTION_SET, lcdconst.FS_DL | fn)
	sleep(0.05)
	self.command(lcdconst.FUNCTION_SET, lcdconst.FS_DL | fn)
	sleep(0.045)
	self.command(lcdconst.DISPLAY_ONOFF, 0x00) #Display off, cursor off, blinking off
	sleep(0.045)
	self.command(lcdconst.CLEAR_DISPLAY, 0x00)
	sleep(0.06)
	self.command(lcdconst.ENTRY_MODE_SET, lcdconst.EMS_ID) #Increment by 1, no shift
	sleep(0.3)
	self.command(lcdconst.DISPLAY_ONOFF, lcdconst.DOO_D) #Display ON
	sleep(0.06)
	self.waitflag = 1

    def power(self, power):
	self.cpot.power(power)
	self.bpot.power(power)
	self.setControl(lcdconst.PWR, power)
	if self.control & lcdconst.PWR:
	    self.init()

    def setContrast(self, value):
	self.cpot.set(value)

    def setBacklight(self, value):
	self.bpot.set(value)

    def blink(self, value):
	self.command(lcdconst.DISPLAY_ONOFF, (self.commands[lcdconst.DISPLAY_ONOFF] | lcdconst.DOO_B) if value else (self.commands[lcdconst.DISPLAY_ONOFF] & (~lcdconst.DOO_B)))

    def cursor(self, value):
	self.command(lcdconst.DISPLAY_ONOFF, (self.commands[lcdconst.DISPLAY_ONOFF] | lcdconst.DOO_C) if value else (self.commands[lcdconst.DISPLAY_ONOFF] & (~lcdconst.DOO_C)))

    def display(self, value):
	self.command(lcdconst.DISPLAY_ONOFF, (self.commands[lcdconst.DISPLAY_ONOFF] | lcdconst.DOO_D) if value else (self.commands[lcdconst.DISPLAY_ONOFF] & (~lcdconst.DOO_D)))

    def test(self):
	self.iface.setPortDir(lcdconst.CPORT, 0x00)
	self.iface.setPortDir(lcdconst.DPORT, 0x00)
	self.iface.setPortOutput(lcdconst.CPORT, 0x00)
	self.iface.setPortOutput(lcdconst.DPORT, 0x00)
	ports = {lcdconst.CPORT: {0 : "E", 1 : "R/W", 2 : "R/S", 3 : "IRS", 4 : "PC", 5 : "UD", 6 : "CCS", 7 : "BCS"},
		 lcdconst.DPORT: {0 : "D0", 1 : "D1", 2 : "D2", 3 : "D3", 4 : "D4", 5 : "D5", 6 : "D6", 7 : "D7"}}
	for port, bits in ports.iteritems():
	    for bit, label in bits.iteritems():
		print "Port {0}; Bit {1}; Label {2}".format(port, bit, label)
		self.iface.setPortOutput(port, 1 << bit)
		self.wait(0.5)
	    self.iface.setPortOutput(port, 0x00)

	self.iface.setPortOutput(lcdconst.CPORT, 1 | 2 | 4)
	self.wait(3)
	self.iface.setPortOutput(lcdconst.CPORT, 0x00)
	self.iface.setPortOutput(lcdconst.DPORT, 0xFF)
	self.wait(3)
	self.iface.setPortOutput(lcdconst.DPORT, 0x00)

	self.iface.setPortDir(lcdconst.CPORT, 0xFF)
	self.iface.setPortDir(lcdconst.DPORT, 0xFF)

	self.iface.setPortOutput(lcdconst.CPORT, 1 | 2 | 4)
	self.wait(1)
	self.iface.setPortOutput(lcdconst.CPORT, 0x00)

	self.iface.setPortOutput(lcdconst.DPORT, 0xFF)
	self.wait(1)
	self.iface.setPortOutput(lcdconst.DPORT, 0x00)

