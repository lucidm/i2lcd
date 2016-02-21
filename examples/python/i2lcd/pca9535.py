# -*- coding: utf-8 -*-

import smbus

class PCA9535(object):
    INPUT_PORT0 = 0
    INPUT_PORT1 = 1
    OUTPUT_PORT0 = 2
    OUTPUT_PORT1 = 3
    POLINV0 = 4
    POLINV1 = 5
    CONF_PORT0 = 6
    CONF_PORT1 = 7

    def __init__(self, bus, address):
	self.bus = smbus.SMBus(bus)
	self.address = address
	#self.ports = range(8)
	#for i in self.ports:
	#    self.ports[i] = self.bus.read_byte_data(self.address, i)

    def setPort(self, port, value):
	self.bus.write_byte_data(self.address, port, value)
	#self.ports[port] = value

    def getPort(self, port):
	ret = self.bus.read_byte_data(self.address, port)
	#self.ports[port] = ret
	return ret

    def setPortDir(self, port, direction):
	self.setPort(self.CONF_PORT0 + port, direction)

    def getPortDir(self, port):
	return self.getPort(self.CONF_PORT0 + port) 

    def setPortOutput(self, port, value):
	self.bus.write_byte_data(self.address, self.OUTPUT_PORT0 + port, value)
	#self.ports[port] = value

    def setPortOutputBlock(self, port, values):
	self.bus.write_block_data(self.address, self.OUTPUT_PORT0 + port, values)

    def getPortOutput(self, port):
	return self.bus.read_byte_data(self.address, self.OUTPUT_PORT0 + port)
	#self.ports[port] = ret
	return ret

    def getPortInput(self, port):
	return self.bus.read_byte_data(self.address, self.INPUT_PORT0 + port)

    def setPortPolarity(self, port, polarity):
	self.setPort(self.POLINV0 + port, polarity)

    def getPortPolarity(self, port):
	return self.getPort(self.POLINV0 + port)
