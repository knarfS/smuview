FROM ubuntu:22.04 AS sigrok-appimage

LABEL org.opencontainers.image.title="SmuView AppImage Multi-Arch Build Image" \
	org.opencontainers.image.description="This image is used to build the SmuView AppImage" \
	org.opencontainers.image.url="https://github.com/knarfS/smuview" \
	org.opencontainers.image.source="https://github.com/knarfS/smuview" \
	org.opencontainers.image.licenses="GPL-3.0-or-later" \
	org.opencontainers.image.authors="Frank Stettner <frank-stettner@gmx.net>" \
	maintainer="Frank Stettner <frank-stettner@gmx.net>"

ENV BASE_DIR=/opt
# When an AppImage is run inside Docker (no FUSE), we need:
ENV APPIMAGE_EXTRACT_AND_RUN=1

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
	&& apt-get upgrade -y \
	&& apt-get install -y --no-install-recommends \
		# Install basic stuff
		wget unzip bzip2 sed \
		# Install build stuff
		git-core cmake gcc g++ make autoconf autoconf-archive automake libtool \
		pkg-config check doxygen swig shellcheck \
		# Install libserialport, libsigrok and smuview dependencies
		python3.10-full libpython3.10-dev \
		# python3-full libpython3-dev
		libglib2.0-dev libglibmm-2.4-dev libzip-dev qtbase5-dev libqt5svg5-dev \
		libusb-1.0-0-dev libftdi1-dev libhidapi-dev libbluetooth-dev libvisa-dev nettle-dev \
		libavahi-client-dev libieee1284-3-dev libboost-dev \
		# Install Qwt dependencies
		mesa-common-dev libgl1-mesa-dev libqt5opengl5-dev \
	# Cleanup apt
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/*

# Install Qwt 6.3.0
RUN cd ${BASE_DIR} \
	&& wget https://sourceforge.net/projects/qwt/files/qwt/6.3.0/qwt-6.3.0.tar.bz2 \
	&& tar xf qwt-6.3.0.tar.bz2 \
	&& cd qwt-6.3.0 \
	# Change the QWT_INSTALL_PREFIX in qwtconfig.pri to /usr
	&& sed -i 's|^\([[:space:]]*QWT_INSTALL_PREFIX[[:space:]]*=[[:space:]]*\)/usr.*$|\1/usr|g' qwtconfig.pri \
	# Disable some stuff to speed up the build
	&& sed -i 's|^\([[:space:]]*QWT_CONFIG[[:space:]]*+=[[:space:]]*QwtDesigner[[:space:]]*\)$|#\1|g' qwtconfig.pri \
	&& sed -i 's|^\([[:space:]]*QWT_CONFIG[[:space:]]*+=[[:space:]]*QwtExamples[[:space:]]*\)$|#\1|g' qwtconfig.pri \
	&& sed -i 's|^\([[:space:]]*QWT_CONFIG[[:space:]]*+=[[:space:]]*QwtPlayground[[:space:]]*\)$|#\1|g' qwtconfig.pri \
	&& sed -i 's|^\([[:space:]]*QWT_CONFIG[[:space:]]*+=[[:space:]]*QwtTests[[:space:]]*\)$|#\1|g' qwtconfig.pri \
	# Build
	&& qmake qwt.pro \
	&& make \
	&& make install \
	# Cleanup
	&& cd .. \
	&& rm qwt-6.3.0.tar.bz2 \
	&& rm -rf qwt-6.3.0
