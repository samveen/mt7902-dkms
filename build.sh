#!/bin/bash -x

echo -n "Build env: "; uname -a
echo "===params==="; echo "'" "$@" "'" ; echo "==========="
echo "===OSInfo==="; cat /etc/os-release ; echo "============"
echo "===ENV==="; env ; echo "========="

# Get and parse options
OPTIONS=$(getopt -o k: --long kver: -n "$0" -- "$@")

# Note the quotes around `$OPTIONS': they are essential!
eval set -- "$OPTIONS"

while true ; do
	case "$1" in
		-k|--kver) kernelver=$2 ; shift 2 ;;
		--) shift ; break ;;
		*) echo "Internal error!" ; exit 1 ;;
	esac
done

if [[ -z ${kernelver} ]]; then
    kernelver=$(uname -r)
    echo "No kernel version. Using current ver by default($kernelver)"
else
    echo "Building for kernel version '$kernelver'"
fi

export kernelver
# build
make -C mt76/mt7902 V=1 VERBOSE=1
