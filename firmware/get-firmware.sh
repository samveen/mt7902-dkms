#!/bin/bash

# Debug logging
set -x

# Get the driver zip listed in the URL file
URL=acer.url
ZIP="$(echo -e "$(basename $(sed 's/\?.*//' < $URL) | sed 'y/+/ /; s/%/\\x/g')")"

wget -O "$ZIP" -i $URL

unzip "$ZIP"

sudo mv -iv "${ZIP//A.zip}(MTK7902)"/*MT7902* /lib/firmware/mediatek/
