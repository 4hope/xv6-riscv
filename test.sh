#!/bin/bash
[ -f "ext2.img" ] && rm -f ext2.img
truncate --size 100M ext2.img
mkfs.ext2 ext2.img > /dev/null

[ -d "ext2" ] && rm -rf ext2
mkdir ext2

mount_point=$(mktemp -d)
loop_dev=$(sudo losetup --find --show ext2.img)
sudo mount "$loop_dev" "$mount_point"

cd "$mount_point" || exit 1
rm -rf ./*
mkdir dir
echo "good morning" > dir/txt
echo "goooood morning" >> dir/txt
truncate --size 2M file
cd - > /dev/null

sha512sum "$mount_point/file" | cut -d ' ' -f1 > real
inode=$(stat -c '%i' "$mount_point/file")
text_inode=$(stat -c '%i' "$mount_point/dir/txt")

sudo umount "$mount_point"
sudo losetup -d "$loop_dev"
rmdir "$mount_point"

gcc -Wall -Wextra -Werror -o main main.c
./main ext2.img "$inode" | sha512sum | cut -d ' ' -f1 > my

echo 
echo "info from file"
./main ext2.img "$text_inode"

echo 
echo "check"
diff -u real my