### wxkb_switch
wxkb_switch - Utility for switching keyboard layouts under Wayland window system.

### Installing 

#### Local install
To install local use:
```
git clone https://github.com/FrolovRuslan1/wxkb_switch.git; cd wxkb_switch; mkdir build; cd build; cmake; make; sudo make install
```
OR
```
git clone https://github.com/FrolovRuslan1/wxkb_switch.git 
cd wxkb_switch
mkdir build
cd build
cmake
make
sudo make install
```

### Unistalling
#### Local
To install local use:
```
git clone https://github.com/FrolovRuslan1/wxkb_switch.git; cd wxkb_switch; mkdir build; cd build; cmake; make; sudo make install; sudo make uninstall
```
OR
```
git clone https://github.com/FrolovRuslan1/wxkb_switch.git 
cd wxkb_switch
mkdir build
cd build
cmake
make
sudo make install
sudo make uninstall
```

#### Using .deb
Download package
`sudo dpkg -i wxkb_switcher-0.0.-Linux.deb`


### .deb package and others
I use cpack to generate .deb package
`sudo cpack -G DEB`
