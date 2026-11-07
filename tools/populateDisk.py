import os
import sys
import subprocess

disk_image = sys.argv[1]
bin_dir = sys.argv[2]

if not os.path.exists(disk_image):
    subprocess.run(["mke2fs", "-b", "1024", "-I", "128", "-F", "-O", "none", "-m", "0", disk_image, "32M"], check=True)

subprocess.run(["debugfs", "-w", "-R", "mkdir bin", disk_image], capture_output=True)

for f in sorted(os.listdir(bin_dir)):
    src = os.path.join(bin_dir, f)
    if os.path.isfile(src):
        subprocess.run(["debugfs", "-w", "-R", f"rm bin/{f}", disk_image], capture_output=True)
        subprocess.run(["debugfs", "-w", "-R", f"write {src} bin/{f}", disk_image], check=True, capture_output=True)

print("Populated disk.img with user binaries in /bin")

