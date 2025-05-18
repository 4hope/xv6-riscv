#! /bin/bash

dev=/dev/loop0

rm -rd ext2.img

truncate --size 100M ext2.img
mkfs.ext2 ext2.img

rmdir ext2
mkdir ext2

mount -o loop -t ext2 ext2.img ext2

cd ext2

rm -r *

mkdir dir

echo "good morning" > dir/txt
echo "goooood morning" >> dir/txt

truncate --size 2M file

cd ..
sha512sum ext2/file | cut -d ' ' -f1 > real

inode=$(stat -c '%i' ext2/file)
text=$(stat -c '%i' ext2/dir/txt)

umount ext2

gcc -Wall -Wextra -Werror -o main main.c

./main ext2.img $inode | sha512sum | cut -d ' ' -f1 > my

echo 
echo "info from file"
./main ext2.img $text

echo 
echo "check"
diff real my

losetup -d $dev