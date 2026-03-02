from socket import socket as Socket
from socket import AF_INET, SOCK_STREAM, timeout
from threading import Thread
from time import sleep


def inputValue():
	global value, running
	while running:
		try:
			val = input("Input: ")
			val = int(val)
		except (EOFError, KeyboardInterrupt):
			print("\nЗавершение програмы...")
			running = False
			continue
		except:
			continue

		if val > 255:
			val = 255
		
		value = val


def main():
	server = Socket(AF_INET, SOCK_STREAM)
	# добавить проверку на bind
	server.bind(("0.0.0.0", 13000))
	server.listen()

	while running:
		# добавить защиту accept
		sock, _ = server.accept()
		Thread(target=conn, args=(sock,), daemon=True).start()


def conn(sock: Socket):
	# добавить защиту, recv|send
	while True:
		data = sock.recv(1)
		if data == b"\xAF":
			sock.send(bytes([value]))
		

if __name__ == "__main__":
	value = 0;
	running = True;
	Thread(target=main, daemon=True).start()
	inputValue()
