#!/bin/bash

MODULE_NAME="mt7902"
MODULE_VERSION="0.0.1"

# Print status
sudo dkms status --verbose -m $MODULE_NAME -v $MODULE_VERSION

# Remove all the previous versions
sudo dkms uninstall --verbose -m $MODULE_NAME -v $MODULE_VERSION
sudo dkms remove --verbose -m $MODULE_NAME -v $MODULE_VERSION --all

# Build and Install the module
sudo dkms build --verbose -m $MODULE_NAME -v $MODULE_VERSION
sudo dkms install --verbose -m $MODULE_NAME -v $MODULE_VERSION

# Print status
sudo dkms status --verbose -m $MODULE_NAME -v $MODULE_VERSION

# remove kernel modules
sudo rmmod mt7902 
sudo rmmod mt7902-common

# reinstall kernel modules
sudo insmod ./mt76/mt7902/mt7902-common.ko
sudo insmod ./mt76/mt7902/mt7902.ko 
