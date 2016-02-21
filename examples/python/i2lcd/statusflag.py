from time import sleep, clock
import lcdconst


class DelayWaiter(object):
    '''
    Simple wait class, based upon time.sleep() method
    '''
    def __init__(self, interface):
	self.iface = interface

    def __call__(self, delay):
	self.wait(delay)

    def wait(self, delay):
	sleep(delay)


class BusyLoop(object):
    '''
    Class uses Busy Flag of an LCD to determine if LCD is done
    '''
    def __init__(self, interface):
	self.iface = interface

    def __call__(self, delay):
	self.delay = delay
	self.ts = clock()
	self.wait()

    def wait(self):
	while (self.iface.readStatus()[0]):
	    continue

class IrqWait(object):
    
    def __init__(self, interface):
	self.iface = interface
	raise ErrorNotImplemented

