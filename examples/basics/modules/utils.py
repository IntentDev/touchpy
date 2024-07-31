import msvcrt

def check_key(key):
	if msvcrt.kbhit():
		char = msvcrt.getch()
		if char in [b'\000', b'\xe0', b'\x00']:
			return False
		
		return char.decode("utf-8") == key
