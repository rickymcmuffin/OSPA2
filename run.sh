make all

sudo insmod lkmasg2.ko
sudo ./test /dev/lkmasg2
sudo rmmod lkmasg2
