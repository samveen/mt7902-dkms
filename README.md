# HELP NEEDED
Please help test the build for this. 

# mt7902-dkms
Create driver for MT7902 by cloning the [MT9721 driver from the kernel](https://github.com/torvalds/linux/tree/master/drivers/net/wireless/mediatek/mt76/mt7921) and modifying it.

## Motivation
https://github.com/keepsoftware/mt7902-dkms/issues/2

## Status
As of now, the code is purely the driver for the mt7921, with just string replacement from mt7921 to mt7902 even in filenames. The build is untested

## Requirements
- Best tested against the [Ubuntu Mainline Kernel](https://wiki.ubuntu.com/Kernel/MainlineBuilds) [v6.8](https://kernel.ubuntu.com/mainline/v6.8/) or greater installed on [Ubuntu 24.04 (Noble Numbat)](https://cloud-images.ubuntu.com/noble/)

- `sudo apt install dkms` should cover all the requirements to test this on ubuntu/debian systems. I'm out of touch with Rocky/Alma/Fedora and cousins.

## Usage instructions

- *(One time setup)* Clone this repo into `/usr/src/`:
  ```
  sudo git clone https://github.com/samveen/mt7902-dkms /usr/src/mt7902-dkms
  ```

- Create a link to the source dir with a version number(easier versioned driver updates):
  ```
  sudo ln -s /usr/src/mt7902-dkms /usr/src/mt7902-dkms-0.0.1
  ```

- Register the module with DKMS:
  ```
  sudo dkms add --verbose -m mt7902-dkms -v 0.0.1
  ```

- Build and install the module:
  ```
  sudo dkms build --verbose -m mt7902-dkms -v 0.0.1   # NEEDS TESTING
  sudo dkms install --verbose -m mt7902-dkms -v 0.0.1 # NEEDS FIXING
  ```

- Check status:
  ```
  sudo dkms status --verbose -m mt7902-dkms -v 0.0.1
  ```

- Get rid of this setup in case it's a pain:
  ```
  sudo dkms uninstall --verbose -m mt7902-dkms -v 0.0.1
  sudo dkms remove --verbose -m mt7902-dkms -v 0.0.1 --all
  sudo rm -fR /usr/src/mt7902-dkms\*
  ```

- Update source
  ```
  cd /usr/src/mt7902-dkms && sudo git pull
  ```

## PCI ID details

- [https://wikidevi.wi-cat.ru/MediaTek\_MT7902\_Reference\_Design](https://wikidevi.wi-cat.ru/MediaTek_MT7902_Reference_Design)
- Windows driver `inf` contents as sources from link in [firmware/acer.url](firmware/acer.url)

## Firmware
Initial firmware source is the acer website URL for the mt7902 windows driver. This needs to be clarified, as it's proprietary.

# License
- For licensing information of the source, please see https://github.com/torvalds/linux/
- For licensing information of firmware, there is no firmware in this repo.
- The contents of URL locations in `firmware/` that are owned by their respective owners. Visit their websites for licensing information.
