cmd_/home/ppw/Documents/LKM-practice/globalmem_lock/globalmem_lock.ko := ld -r -m elf_x86_64  -z max-page-size=0x200000  --build-id  -T ./scripts/module-common.lds -o /home/ppw/Documents/LKM-practice/globalmem_lock/globalmem_lock.ko /home/ppw/Documents/LKM-practice/globalmem_lock/globalmem_lock.o /home/ppw/Documents/LKM-practice/globalmem_lock/globalmem_lock.mod.o;  true