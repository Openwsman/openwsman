
from OWsman import *
from AMT import *
#import pdb

#cl = WsmanClient("bl003", 16992, "/wsman", "http", "admin", "Admin@98")
cl = WsmanClient("bl004", 8889, "/wsman", "http", "wsman", "secret")


print "#################################"
#pdb.set_trace()
cs = CIM_Process(cl)
lcs = cs.Enumerate()
for c in lcs:
	c.dump()
