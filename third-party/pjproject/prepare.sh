#!/bin/bash -ex
# inspired by
# https://github.com/asterisk/asterisk/blob/master/third-party/pjproject/Makefile.rules
#

TARBALL_FILE="pjproject-2.8.tar.bz2"
TARBALL_MD5="$TARBALL_FILE.md5"
PACKAGE_URL="https://www.pjsip.org/release/2.8/$TARBALL_FILE"
UNPACK_DIR="pjproject-2.8"

# download
if [ ! -f "$TARBALL_FILE" ]; then
  wget $PACKAGE_URL
fi

# verify tarball
tarball_sum=`md5sum $TARBALL_FILE | cut -d ' ' -f1`
required_sum=`grep -e $TARBALL_FILE $TARBALL_MD5 | cut -d ' ' -f1`
if [ -z "$required_sum" -o "$tarball_sum" != "$required_sum" ] ; then
  echo "verify $TARBALL_FILE failed"
  exit 1
fi

# extract
if [ ! -d "$UNPACK_DIR" ]; then
  tar xfj $TARBALL_FILE
fi

# configuration
# even though we're not installing pjproject, we're setting prefix to /opt/pjproject to be safe
CONFIG_OPTS="--prefix=/opt/pjproject \
  --disable-speex-codec \
  --disable-speex-aec \
  --disable-gsm-codec \
  --disable-ilbc-codec \
  --disable-g7221-codec \
  --disable-ipp \
  --disable-sound \
  --disable-ext-sound \
  --disable-opencore-amr \
  --disable-silk \
  --disable-opus \
  --disable-sdl \
  --disable-video \
  --disable-v4l2 \
  --disable-libyuv \
  --disable-ffmpeg \
  --disable-openh264 \
  --disable-libwebrtc \
  --without-external-pa \
  --without-external-srtp \
  --disable-resample \
  --enable-ssl"
# Linux  
CONFIG_OPTS+="--enable-epoll"

# build
cd $UNPACK_DIR
./configure $CONFIG_OPTS
make all
