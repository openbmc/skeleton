GPIO_BASE = 320

def convertGpio(name):
    name = name.upper()
    c = name[0:1]
    offset = int(name[1:])
    a = ord(c)-65
    base = a*8+GPIO_BASE
    return base+offset
