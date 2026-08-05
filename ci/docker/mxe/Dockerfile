ARG BASE_DIR=/opt
ARG MXE_DIR=$BASE_DIR/mxe


FROM debian:trixie AS sigrok-mxe-builder

ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update \
	&& apt-get upgrade -y \
	&& apt-get install -y --no-install-recommends \
		sudo \
		autoconf \
		automake \
		autopoint \
		bash \
		bison \
		bzip2 \
		flex \
		g++ \
		g++-multilib \
		gettext \
		git \
		gperf \
		gtk-doc-tools \
		intltool \
		libc6-dev-i386 \
		libclang-dev \
		libgdk-pixbuf-xlib-2.0-dev \
		libltdl-dev \
		libssl-dev \
		libtool-bin \
		libxml-parser-perl \
		lzip \
		make \
		openssl \
		p7zip-full \
		patch \
		perl \
		pkg-config \
		python3 \
		python3-mako \
		python3-packaging \
		python3-pkg-resources \
		python3-setuptools \
		python-is-python3 \
		ruby \
		sed \
		unzip \
		wget \
		xz-utils \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/*

ARG BASE_DIR
ARG MXE_DIR
ENV MXE_TARGETS="x86_64-w64-mingw32.static.posix"

WORKDIR ${BASE_DIR}
COPY check.mk.patch ${BASE_DIR}
COPY qwt.mk.patch ${BASE_DIR}
COPY libusb1.mk.patch ${BASE_DIR}

RUN git clone https://github.com/mxe/mxe.git ${MXE_DIR}

WORKDIR ${MXE_DIR}

# Pin MXE to commit f7c7eb313d39252c6dfd5b09a2ae7d164fc7a5a2
# For info about the patches, see commit messages in the patch files
RUN git reset --hard f7c7eb313d39252c6dfd5b09a2ae7d164fc7a5a2 \
	&& patch -p1 < ${BASE_DIR}/check.mk.patch \
	&& patch -p1 < ${BASE_DIR}/qwt.mk.patch \
	&& patch -p1 < ${BASE_DIR}/libusb1.mk.patch

RUN make -j"$(nproc)" MXE_USE_CCACHE= DONT_CHECK_REQUIREMENTS=1 MXE_VERBOSE=1 MXE_TARGETS="${MXE_TARGETS}" \
		gcc \
		glib \
		libzip \
		libusb1 \
		libftdi1 \
		hidapi \
		glibmm \
		qtbase \
		qtimageformats \
		qtsvg \
		qttranslations \
		boost \
		check \
		gendef \
		libieee1284 \
		nettle \
		qwt \
		qtbase_CONFIGURE_OPTS='-no-sql-mysql' \
	&& make clean-junk \
	&& rm -rf ${MXE_DIR}/pkg \
	&& rm -rf ${MXE_DIR}/.git


FROM debian:trixie-slim AS sigrok-mxe

LABEL org.opencontainers.image.title="SmuView MXE Build Image" \
	org.opencontainers.image.description="This image is used to cross-compile SmuView for Windows with MXE" \
	org.opencontainers.image.url="https://github.com/knarfS/smuview" \
	org.opencontainers.image.source="https://github.com/knarfS/smuview" \
	org.opencontainers.image.licenses="GPL-3.0-or-later" \
	org.opencontainers.image.authors="Frank Stettner <frank-stettner@gmx.net>" \
	maintainer="Frank Stettner <frank-stettner@gmx.net>"

# NOTE: sdcc is not installed to safe space. The firmware from sigrok-util is not compiled for SmuView
ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update \
	&& apt-get upgrade -y \
	&& apt-get install -y --no-install-recommends \
		autoconf \
		automake \
		ca-certificates \
		doxygen \
		git \
		libtool-bin \
		m4 \
		make \
		nsis \
		pkg-config \
		python3 \
		python-is-python3 \
		sed \
		unzip \
		vim-tiny \
		wget \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/*

ARG BASE_DIR
ARG MXE_DIR
ENV BASE_DIR=${BASE_DIR}
ENV MXE_DIR=${MXE_DIR}

COPY --from=sigrok-mxe-builder ${MXE_DIR} ${MXE_DIR}

WORKDIR ${MXE_DIR}
