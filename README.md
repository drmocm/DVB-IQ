# DVB-IQ
small example programs in C and PYTHON to show IQ data with libdddvb and 
DigitalDevices MAX SX8

**For the C example**

You have to build dvb_iq with

`make` 

You need to install the libdddvb library 

https://github.com/DigitalDevices/dddvb

and GTK 3.0 libraries as well as dvben50221.


On an Ubuntu system this would look like this:

`sudo apt-get install build-essential libgtk-3-dev libglib2.0-dev pkg-config dvb-apps` 

`git clone https://github.com/DigitalDevices/internal_dddvb.git` 

`cd internal_dddvb/lib/; make` 

`sudo make install` 

`cd`

`git clone https://github.com/drmocm/DVB-IQ.git` 

`cd DVB-IQ` 

`make` 


(Tell me if I forgot any)

A typical example call would be:

`./dvb_iq -d S2 -p v -f 10847000  -s 23000000  -c ~/ddzapconf/ -l64 -i 0x10000000` 

this visualizes the output you would get from: 

`ddzap -d S2 -p v -f 10847000  -s 23000000  -c ~/ddzapconf/ -l64 -i 0x10000000 -o` 

Additionally you can use dvb_iq with any program that gives you IQ symbols in
8bit integer format, e.g. with ddzap that would be:

`ddzap -d S2 -p v -f 10847000  -s 23000000  -c ~/ddzapconf/ -l64 -i 0x10000000 -o | ./dvb_iq` 


tuning parameters are for astra 28°E with my unicable setup and the lnb 
connected to the second input of my MAX SX8 with your LNB config file in
~/ddzapconf/

Ther is one more cli option for chosing the color scheme:

`./dvb_iq -q X`

selects scheme number X (1=red, 2=green, 3=blue, 4= multi color)

**For the PYTHON example**

Just use iqtk.py e.g. as follows:

`ddzap -d S2 -p v -f 10847000  -s 23000000  -c ~/ddzapconf/ -l64 -i 0x10000000 -o | ./iqtk.py` 


