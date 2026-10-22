import struct
import sys

MAGIC = b"KANHA"
HEADER_SIZE = 512

kernelPath = sys.argv[1]
headerPath = sys.argv[2]

with open(kernelPath, "rb") as f:
    kernelSize = len(f.read())

header = bytearray(HEADER_SIZE)

header[0:5] = MAGIC
struct.pack_into("<I", header, 5, kernelSize)

with open(headerPath, "wb") as f:
    f.write(header)
