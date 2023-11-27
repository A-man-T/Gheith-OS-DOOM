import os
import time
from subprocess import Popen, PIPE

# Usage: `python3 tests/deviceIO_test/serial_stdin.dir/test_run_serial_stdin.py`

def run_qemu(test_name: str):
    build = Popen(
        f"UTCS_OPT=-O3 make clean the_kernel {test_name}.build {test_name}.data", shell=True)
    build.wait()
    process = Popen(f"`make qemu_cmd` `make qemu_config_flags` \
             -no-reboot \
             -nographic \
             --monitor none \
             -d guest_errors \
             -D qemu.log \
             -drive file=kernel/build/kernel.img,index=0,media=disk,format=raw \
             -drive file={test_name}.data,index=1,media=disk,format=raw \
             -device isa-debug-exit,iobase=0xf4,iosize=0x04 || true", shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    return process


def test_serial_input():
    with open("tests/deviceIO_test/serial_stdin.dir/files/sample_input.txt", "rb") as infile:
        sample_input = infile.read()
        process = run_qemu("deviceIO_test.serial_stdin")
        # Have to add some delay otherwise will get consumed before our OS is ready for it
        time.sleep(1)
        stdout, stderr = process.communicate(input=sample_input)
        output = stdout.decode().split()
        assert output[-1] == "shutdown"
        assert output[-3] == "leaks"
        assert output[-4] == "frame"
        assert output[-6] == "leaks"
        assert output[-7] == sample_input.decode().split()[0]
        print("\n\n*** PASSED ***\n\n")

if __name__ == "__main__":
    test_serial_input()