# wxkb_switch
wxkb_switch - Utility for switching keyboard layouts under Wayland window system.

### Installing 

#### Local install
To install local use:
```
sudo apt install libx11-dev libxkbcommon-x11-dev libxkbfile-dev libxcb1-dev; git clone https://github.com/FrolovRuslan1/wxkb_switch.git; cd wxkb_switch; mkdir build; cd build; cmake; make; sudo make install
```
OR
```
sudo apt install libx11-dev libxkbcommon-x11-dev libxkbfile-dev libxcb1-dev
git clone https://github.com/FrolovRuslan1/wxkb_switch.git 
cd wxkb_switch
mkdir build
cd build
cmake ..
make
sudo make install
```
#### Using .deb
`wget https://github.com/FrolovRuslan1/wxkb_switch/releases/download/1.0.0/wxkb_switcher-1.0.0-Linux.deb`

`sudo apt install ./wxkb_switcher-0.0.-Linux.deb`

### Unistalling
#### Local
To install local use:
```
sudo apt install libx11-dev libxkbcommon-x11-dev libxkbfile-dev libxcb1-dev; git clone https://github.com/FrolovRuslan1/wxkb_switch.git; cd wxkb_switch; mkdir build; cd build; cmake; make; sudo make install; sudo make uninstall
```
OR
```
sudo apt install libx11-dev libxkbcommon-x11-dev libxkbfile-dev libxcb1-dev
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
`sudo apt remove wxkb_switcher`


### .deb package and others
I use cpack to generate .deb package
`sudo cpack -G DEB`
