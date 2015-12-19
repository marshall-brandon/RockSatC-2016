import sys
import smbus
import math
import time
import RPi.GPIO as GPIO

def detector_setup(pins):
	''' Sets a list of detector pins to input mode. '''
	GPIO.setmode(GPIO.BOARD)	# clarify which pin nums to use
	for p in pins:
		GPIO.setup(p, GPIO.IN)	# set all pins as inputs

def read_byte(i2c, reg):
	return bus.read_byte_data(i2c, reg)

def read_word(i2c, reg):
	high = bus.read_byte_data(i2c, reg)
	low  = bus.read_byte_data(i2c, reg)
	return (high << 8) + low

def read_word_2c(i2c, reg):
	val = read_word(i2c, reg)
	if val >= 0x8000:
		return -(65535-val+1)
	else:
		return val

def dist(a,b):
	return math.sqrt((a*a)+(b*b))

def get_y_rotation(x,y,z):
	radians = math.atan2(x, dist(y,z))
	return -math.degrees(radians)

def get_x_rotation(x,y,z):
	radians = math.atan2(y, dist(x,z))
	return math.degrees(radians)

def read_mpu(addr):
	''' Returns a list of MPU data. (ax,ay,z,temp,gx,gy,gz) '''
	data = bus.read_i2c_block_data(addr, 0x3b, 14)
	ax = (data[0] << 8) + data[1]
	ay = (data[2] << 8) + data[3]
	az = (data[4] << 8) + data[5]
	temp = (data[6]<< 8) + data[7]
	gx = (data[8]  << 8) + data[9]
	gy = (data[10] << 8) + data[11]
	gz = (data[12] << 8) + data[13]
	return ax, ay, az, temp, gx, gy, gz

def read_detectors(pins):
	''' Takes a list of input pins, returns their values. '''
	return [GPIO.input(p) for p in pins]

def write_to_sd(data, filename="test_output.txt"):
	''' Appends data to an output file. '''
	f = open(filename, 'a')
	for line in data:
		f.write(line)
	f.close()

def benchmark(mpu_addr, det_pins, num_loops=100, test_size=100):
	''' Simulates filling the buffer, then displays time. '''
	buffer = [None] * test_size

	print "Running {} cycles of a buffer with {} positions.".format(num_loops, test_size)

	start_time = time.time()

	for loop in range(num_loops):
		for i in range(test_size):
			# Replace buffer contents with timestamp and data
			buffer[i] = str(time.time())+','
		        buffer[i] += ','.join(str(x) for x in read_mpu(mpu_addr))
		        buffer[i] += ','.join(str(x) for x in read_detectors(det_pins))
			buffer[i] += '\n'
		# Flush buffer to SD card
		write_to_sd(buffer, "benchmark_output.txt")

	end_time = time.time()

	# Display last few lines of buffer just for debugging
	for line in buffer[:10]:
	        print line

	total_time = end_time-start_time
	print "Start time: {}, end time: {}, total time: {:.4}, or {:.4} seconds per line.".format(
	        start_time,end_time,end_time-start_time,
		(end_time-start_time)/(num_loops*test_size))



if __name__ == "__main__":
	# Prepare some default variables
	bus = smbus.SMBus(1)
	det_pins = (11, 12, 13, 15, 18, 22, 29, 31, 32, 33, 35, 36)
	mpu_addr = 0x68	
	mag_addr = 0x04

	# Initialize the detector pins
	detector_setup(det_pins)
	# mpu6050 defaults to power saving mode, we need to wake it up
	bus.write_byte_data(mpu_addr, 0x6b, 0)

	
	benchmark(mpu_addr, det_pins)
