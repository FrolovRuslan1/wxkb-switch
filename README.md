# wxkb_switch
wxkb_switch - Utility for switching keyboard layouts under Wayland window system.
Works under X11 window system too.

### Using
To switch layout to next under Wayland window system use:

`wxkb_switch` or `wxkb_switch --next` or `wxkb_switch -n`

To switch layout to previos under Wayland window system use:

`wxkb_switch --prev` or `wxkb_switch -p`

To list avalible and current layouts use:

`wxkb_switch --list` or `wxkb_switch -l`

IF you want display debug information use `--debug` or `-d` option with anoher options

### Installing 

#### Local install with compiling
To install local use:
```
sudo apt install pkg-config libx11-dev libxkbcommon-x11-dev libxkbfile-dev libxcb1-dev
git clone https://github.com/FrolovRuslan1/wxkb_switch.git 
cd wxkb_switch
mkdir build
cd build
cmake ..
make
sudo make install
```
#### Using .deb
```
wget https://github.com/FrolovRuslan1/wxkb_switch/releases/download/1.0.2/wxkb_switcher-1.0.2-Linux.deb
sudo apt install ./wxkb_switcher-0.0.-Linux.deb
```

### Unistalling
#### Local
```
sudo apt install pkg-config libx11-dev libxkbcommon-x11-dev libxkbfile-dev libxcb1-dev
git clone https://github.com/FrolovRuslan1/wxkb_switch.git 
cd wxkb_switch
mkdir build
cd build
cmake ..
make
sudo make install
sudo make uninstall
```

#### Using .deb
`sudo apt remove wxkb_switcher`

### Supporting
Doesnt support Gnome under X11, but under Wayland supports


### .deb package and others
I use cpack to generate .deb package
`sudo cpack -G DEB`
