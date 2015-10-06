import pprint
import select
import serial
import socket 
import sys  
import cwiid
import time
import RPi.GPIO as GPIO


# program setting variables
HC_HOST = '' # Blank name binds to all available interfaces
HC_PORT = 0xCB34 # Decimal 52020, hex just for laughs
HC_MAXSIZE = 1024
HC_TIMEOUT = 5

# internal global variables (yeah yeah but it' python)
running = True
zigbee_cmds = ['0','1','2','3','4','5','6','7','S','F','N','M','D','A','R','T','O']
local_cmds = ['0','1','2','3','4','5','6','7','S']
client_list = []

pi_pins = [21,22,23,24,25,26,27,28]
pi_states = [0,0,0,0,0,0,0,0]


# device specifications
devices = {}
devices['local'] = {}
devices['local']['relays'] = {}
for i in range(8):
        devices['local']['relays'][i] = 'Off'
devices['local']['relays']['show'] = 'Off'
devices['zb1'] = {}
devices['zb1']['relays'] = {}
for i in range(8):
	devices['zb1']['relays'][i] = 'Off'
devices['zb1']['relays']['show'] = 'Off'
devices['zb1']['motor'] = {
	'spinning': False,
	'oscillation': False,
	'direction': 'Forward/Open',
	'posistion': 0
}

# utility methods
def draw_screen():
	os.system('clear')
	pprint.pprint(devices)

def validate(cmd, valids):
	if cmd in valids:
		return True
	else:
		return False

def local_cmd(cmd):
	if cmd >= 8 or cmd < 0:
		return False
	pi_states[cmd] = not pi_states[cmd]
	GPIO.output(cmd,pi_states[cmd])

def zb_cmd(cmd):
	z.write(cmd)
	ack = z.readline()

def zb_status():
	z.write('Z')
	ack = z.readline()
	status = z.readline()
	return status
		
def current_socket_list():
	current_list = []
	for cl in client_list:
		current_list.append(cl[0])
	return current_list

def client_handler(conn):
	pass

#Set up Wiimote over Bluetooth
#print("Press 1 + 2 on Wiimote now...")
#time.sleep(1)

#try:
#	wii = cwiid.Wiimote()
#except RuntimeError:
#	print("Error settig up wiimote.")
#	quit()

#wii.rpt_mode = cwiid.RPT_BTN

#wii.rumble = 1
#time.sleep(1)
#wii.rumble = 0
#wii.led = 1

#print("Wiimote setup complete!")

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
for pin in pi_pins:
	GPIO.setup(pin, GPIO.OUT)
print("Local GPIO setup done.")


# Create IP Socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
try:
	s.bind( (HC_HOST, HC_PORT) )
except socket.error as msg:
	print( "Socket bind failed. Code: " + str(msg[0]) + ' Msg: ' + msg[1])
	sys.exit()
client_list.append((s, (HC_HOST, HC_PORT)))
s.listen(5)

# Open ZigBee TTY as a serial device
z = serial.Serial('/dev/ttyUSB0', 9600, HC_TIMEOUT)
print("Setup complete.")

# main loop
try:
	while running:
		readable, writable, errored = select.select(current_socket_list(), [], [])
		for sock in readable:
			if sock is s:
				client_socket, address = s.accept()
				client_list.append( (client_socket, address) )
				print("Connection opened", address)
			else:
				data = sock.recv(HC_MAXSIZE)
				if data:
					if data[0] == 'L':
						if validate(data[1], local_cmds):
							print("Valid local command.")
							# TODO local devices?
							local_cmd(data[1])
							sock.sendall('#' + data)
						else:
							sock.sendall('!' + data)
					elif data[0] == 'Z':
						if validate(data[1], zigbee_cmds):
							print("CMD", data)
							zb_cmd(data[1])
							# TODO actually store device state
							sock.sendall('#' + data)
						elif data[1] == 'Z':
							print("ZCMD", data)
							status = zb_status()
							# TODO refresh remote device state
							sock.sendall('#' + data)
							sock.sendall(status)
						else:
							sock.sendall('!' + data)
					elif data[0] == 'H':
						# TODO send entire house state
						pass
					elif data[0] == 'X':
						sock.sendall('#' + data)
						running = False
					else:
						sock.sendall('!' + data)
				else:
					for cl in client_list:
						if cl[0] == sock:
							client_list.remove(cl)
							print ("Connection closed", cl[1])
							break
					sock.close()
		# process keyboard / mouse events here?
		#buttons = wii.state['buttons']
		#if (bool( buttons & cwiid.BTN_A) ):
		#	print("WCMD: TOGGLE")
		#	zb_cmd("A")
finally:
	# Close zigbee serial and IP socket
	s.close()
	z.close()
	print("All devices closed, program terminated.")
