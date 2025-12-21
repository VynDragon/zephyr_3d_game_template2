import math

LENGTH = 512
step = math.pi*2/LENGTH

for i in range(0, LENGTH):
	print("{}*L3_F,".format(math.sin(step * i)))



