import lcdconst
from pca9535 import PCA9535

class Potentiometer(object):
    '''
    Implementation of Up/Down protocol for MCP401x electronic potentiometers
    used for contrast/backlight control
    '''
    def __init__(self, interface, cs, ud):
	self.interface = interface
	direction = self.interface.getPortDir(lcdconst.CPORT) & ~(cs | ud)
	self.interface.setPortDir(lcdconst.CPORT, direction)
	self.control = self.interface.getPortOutput(lcdconst.CPORT) | cs | ud
	self.interface.setPortOutput(lcdconst.CPORT, self.control)
	self.current = 0x1F
	self.csb = cs
	self.udb = ud

    def cs(self, value):
	self.control = self.control | self.csb if value else self.control & ~self.csb
	self.interface.setPortOutput(lcdconst.CPORT, self.control)

    def ud(self, value):
	self.control = self.control | self.udb if value else self.control & ~self.udb
	self.interface.setPortOutput(lcdconst.CPORT, self.control)

    def power(self, power):
	if power:
	    direction = self.interface.getPortDir(lcdconst.CPORT) & ~(self.csb | self.udb)
	    self.interface.setPortDir(lcdconst.CPORT, direction)
	    self.interface.setPortOutput(lcdconst.CPORT, self.csb)
	    self.set(0x00)

    def inc(self):
	self.current += 1 if self.current < 0x3F else 0
	self.ud(1)
	self.cs(0)
	self.ud(0)
	self.ud(1)
	self.cs(1)


    def dec(self):
	self.current -= 1 if self.current > 0 else 0
	self.ud(0)
	self.cs(0)
	self.ud(1)
	self.ud(0)
	self.cs(1)

    def set(self, value):
	value %= 64
	diff = 0
	if value > self.current:
	    diff = value - self.current
	    self.ud(1)
	    self.cs(0)
	    for i in range(diff):
		self.ud(0)
		self.ud(1)
	    self.cs(1)
	    if (self.current + diff) > 0x3F:
		self.current = 0x3F
	    else:
		self.current += diff
	elif value < self.current:
	    diff = self.current - value
	    self.ud(0)
	    self.cs(0)
	    for i in range(diff):
		self.ud(1)
		self.ud(0)
	    self.cs(1)
	    if (self.current - diff) < 0:
		self.current = 0
	    else:
		self.current = self.current - diff
