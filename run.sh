make all

sudo insmod charkmod-in.ko
sudo insmod charkmod-out.ko
sudo python3 test.py 
sudo rmmod charkmod_in
sudo rmmod charkmod_out
