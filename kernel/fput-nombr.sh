#!/bin/sh

mkdir -p /tmp/mnt/vhd01
mount -o loop $1 /tmp/mnt/vhd01
cp -f $2 /tmp/mnt/vhd01/$3
umount /tmp/mnt/vhd01
