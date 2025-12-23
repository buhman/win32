import sys

with open(sys.argv[1], 'rb') as f:
    buf = f.read()

with open(sys.argv[2], 'wb') as f:
    for i in range(len(buf) // 4):
        r = buf[i*4+0]
        g = buf[i*4+1]
        b = buf[i*4+2]
        a = buf[i*4+3]

        f.write(bytes(reversed([a, r, g, b])))
