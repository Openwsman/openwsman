
from OWsman import *
from AMT import *


cl = WsmanClient()
cl.create("bl004", 8889, "/wsman", "http", "wsman", "secret")


print "#################################"
cs = CIM_Process(cl)
lcs = cs.Enumerate()
for c in lcs:
	c.dump()
