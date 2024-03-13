# Motivation
https://github.com/keepsoftware/mt7902-dkms/issues/2

# mt7902-dkms
Create driver for MT7902 by cloning the [MT9721 driver from the kernel](https://github.com/torvalds/linux/tree/master/drivers/net/wireless/mediatek/mt76/mt7921) and modifying it.

# Firmware
Initial firmware source is the acer website URL for the mt7902 windows driver. This needs to be clarified, as it's proprietary.

# Usage instructions

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

# License
- For licensing information of the source, please see https://github.com/torvalds/linux/
- For licensing information of firmware, there is no firmware in this repo.
- The contents of URL locations in `firmware/` that are owned by their respective owners. Visit their websites for licensing information.
