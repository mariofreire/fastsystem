#!/bin/sh

cp -f fskrnl iso
cp -f fskrnl.elf iso/boot
genisoimage -input-charset utf-8 -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o grub.iso iso
