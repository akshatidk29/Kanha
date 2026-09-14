import struct
import sys

MAGIC = b"KNHA"
HEADER_SIZE = 512

kernelPath = sys.argv[1]
headerPath = sys.argv[2]

with open(kernelPath, "rb") as f:
    kernelSize = len(f.read())

header = bytearray(HEADER_SIZE)

header[0:4] = MAGIC
struct.pack_into("<I", header, 4, kernelSize)

with open(headerPath, "wb") as f:
    f.write(header)
