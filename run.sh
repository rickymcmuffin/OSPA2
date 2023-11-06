make all

sudo insmod lkmasg1.ko
sudo ./test /dev/lkmasg1
sudo rmmod lkmasg1
