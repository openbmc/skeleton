GPIO_BASE = 320

def convertGpio(name):
    offset = int(filter(str.isdigit, name))
    port = filter(str.isalpha, name.upper())
    a = ord(port[-1]) - ord('A')
    if len(port) > 1:
        a += 26
    base = a * 8 + GPIO_BASE
    return base + offset
