#!/bin/sh

if [ -z "$1" ]; then
   echo ""
   exit 1
else
   if [ -z "$2" ]; then
      echo ""
      exit 1
   else
      if [ -z "$3" ]; then
         echo ""
         exit 1
      else
         export TMP_IMG_MOUNT_POINT="/tmp/mnt/vhd01"
         export TMP_IMG_MOUNT_DEV=`losetup -Pf $1 --show`
         export TMP_IMG_MOUNT_DEV_POINT="${TMP_IMG_MOUNT_DEV}p1"
         #echo $TMP_IMG_MOUNT_DEV
         #echo $TMP_IMG_MOUNT_DEV_POINT
         sleep 1
         mkdir -p $TMP_IMG_MOUNT_POINT
         mount $TMP_IMG_MOUNT_DEV_POINT $TMP_IMG_MOUNT_POINT
         cp $2 $TMP_IMG_MOUNT_POINT/$3
         umount $TMP_IMG_MOUNT_POINT
         losetup -d $TMP_IMG_MOUNT_DEV
         unset TMP_IMG_MOUNT_DEV_POINT
         unset TMP_IMG_MOUNT_DEV
         unset TMP_IMG_MOUNT_POINT
      fi
   fi
fi

