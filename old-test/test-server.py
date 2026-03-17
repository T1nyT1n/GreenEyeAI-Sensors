import struct
from functools import wraps

from socket import socket as Socket
from socket import AF_INET, SOCK_STREAM, timeout

from threading import Thread


# декоратор для автоматизации написания потоковых функций
def threaded(func):
	@wraps(func)
	def decor(*args, **kwargs) -> Thread:
		thread = Thread(target=func, args=args, kwargs=kwargs, daemon=True)
		thread.start()
		return thread
	return decor


# функция для ввода значений
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


@threaded
def main():
	global running
	server = Socket(AF_INET, SOCK_STREAM)
	try:
		server.bind(("0.0.0.0", 13000))
	except:
		print("Не верный адрес или порт уже занят")
		running = False
		return
	server.listen()

	while running:
		sock, _ = server.accept()
		conn(sock) # threaded


@threaded
def conn(sock: Socket):
	running = True
	while running:
		try:
			data = sock.recv(1)
		except:
			running = False
			continue

		if data == b"\xAF":
			data = sock.recv(4)
			x = struct.unpack('f', data)[0]
			print(x);
			try:
				sock.send(b"\xFA" + bytes([value]))
			except:
				running = False


if __name__ == "__main__":
	value = 0;
	running = True;
	main() # threaded
	inputValue()