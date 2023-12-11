make all

sudo insmod lkmasg2.ko
sudo python3 test_pa2.py lkmasg2 1024
sudo rmmod lkmasg2
