#! /usr/bin/env python
import smbus
import time
b=smbus.SMBus(1)
addr = [0x29, 0x39, 0x49]
for i in xrange(3):
    b.write_byte_data(0x70, 0x00, 1 << i)
    for ad in addr:
        b.write_byte_data(ad, 0x80, 0x03)
        print "Power up 0x%x: 0x%x"%(ad,b.read_byte_data(ad, 0x80))
    for ad in addr:
        b.write_byte_data(ad, 0x81, 0x00)
        print "Timing 0x%x: 0x%x"%(ad,b.read_byte_data(ad, 0x81))

# while True:
#     for i in xrange(3):
#         b.write_byte_data(0x70, 0x00, 1 << i)
#         for ad in addr:
#             print b.read_i2c_block_data(ad, 0x9b, 4)