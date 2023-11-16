import os
import time
from subprocess import Popen, PIPE

# I can't figure out a way to run this with the vscode Python extension directly since it
# expects TERM environment variable to be set to a terminal for it to clear
# So everyone will have to run through the following:
# pytest manual_tests/test_run_manuals.py
# may have to do the following if you don't have pytest: pip install -U pytest


def run_qemu(test_name: str):
    build = Popen(
        f"UTCS_OPT=-O3 TESTS_DIR=manual_tests make clean the_kernel {test_name} {test_name}.data", shell=True)
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
    with open("manual_tests/serial_stdin.dir/files/sample_input.txt", "rb") as infile:
        sample_input = infile.read()
        process = run_qemu("serial_stdin")
        # Have to add some delay otherwise will get consumed before our OS is ready for it
        time.sleep(1)
        stdout, stderr = process.communicate(input=sample_input)
        output = stdout.decode().split()
        assert output[-1] == "shutdown"
        assert output[-3] == "leaks"
        assert output[-4] == "frame"
        assert output[-6] == "leaks"
        assert output[-7] == output[-8]
        assert output[-7] == sample_input.decode().split()[0]
