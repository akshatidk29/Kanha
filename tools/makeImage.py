import sys

BOOT_SIZE   = 512
HEADER_SIZE = 512
SECTOR_SIZE = 512

bootPath   = sys.argv[1]
headerPath = sys.argv[2]
kernelPath = sys.argv[3]
imagePath  = sys.argv[4]

with open(bootPath, "rb") as f:
    boot = f.read()

with open(headerPath, "rb") as f:
    header = f.read()

with open(kernelPath, "rb") as f:
    kernel = f.read()

if len(boot) != BOOT_SIZE:
    raise ValueError(
        f"Bootloader must be {BOOT_SIZE} bytes, got {len(boot)}"
    )
if len(header) != HEADER_SIZE:
    raise ValueError(
        f"Header must be {HEADER_SIZE} bytes, got {len(header)}"
    )

payload = header + kernel

# Number of Sectors to store Payload
sectors = (len(payload) + SECTOR_SIZE - 1) // SECTOR_SIZE

# Pad the Payload
payload += b"\x00" * (sectors * SECTOR_SIZE - len(payload))

# Create the Image
image = boot + payload

with open(imagePath, "wb") as f:
    f.write(image)

print(f"Bootloader : {len(boot)} bytes")
print(f"Header     : {len(header)} bytes")
print(f"Kernel     : {len(kernel)} bytes")
print(f"Payload    : {len(payload)} bytes")
print(f"Image      : {len(image)} bytes")