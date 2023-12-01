make all

sudo insmod charkmod-in.ko
sudo insmod charkmod-out.ko
sudo ./test
sudo rmmod charkmod_out
sudo rmmod charkmod_in
