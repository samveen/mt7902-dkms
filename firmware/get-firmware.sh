#!/bin/bash

# Get the driver zip listed in the URL file
URL=acer.url
ZIP=$(basename $(sed 's/\?.*//' < $URL))

wget -O $ZIP -i $URL

unzip $ZIP
