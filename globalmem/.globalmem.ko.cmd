cmd_/home/ppw/Documents/LKM-practice/globalmem/globalmem.ko := ld -r -m elf_x86_64  -z max-page-size=0x200000  --build-id  -T ./scripts/module-common.lds -o /home/ppw/Documents/LKM-practice/globalmem/globalmem.ko /home/ppw/Documents/LKM-practice/globalmem/globalmem.o /home/ppw/Documents/LKM-practice/globalmem/globalmem.mod.o;  true
