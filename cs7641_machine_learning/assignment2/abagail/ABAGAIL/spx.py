import struct
import random

def float_to_bits(fnum):
    hexnum = struct.pack('>f', fnum)
    bits = ''.join([bin(ord(c))
    	     .replace('0b', '')
    	     .rjust(8, '0') 
    	     for c in hexnum])
    return bits

def spxover(f1, f2, k):
    f1_bits = float_to_bits(f1)
    f2_bits = float_to_bits(f2)
    c1_bits = f1_bits[:k] + f2_bits[k:]
    c2_bits = f2_bits[:k] + f1_bits[k:]
    return f1_bits, f2_bits, c1_bits, c2_bits

if __name__ == '__main__':

	f1 = random.random()
	f2 = random.random()
	f1b, f2b, c1b, c2b = spxover(f1, f2, 16)
	print "%s\t%s\n%s\t%s\n" % (f1b, f2b, c1b, c2b)
	if f1b == c1b:
		print "f1b == c1b"

	if f2b == c2b:
		print "f2b == c2b"	
