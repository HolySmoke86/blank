FROM ubuntu:latest

RUN apt update \
	&& apt -y upgrade \
	&& apt -y install --no-install-recommends \
		dbus \
		git \
		make \
		xauth \
		xvfb \
	&& apt-get -y clean \
	;

RUN apt -y install --no-install-recommends \
		libalut0 \
		libglew1.10 \
		libglu1-mesa \
		libopenal1 \
		libsdl2-2.0-0 \
		libsdl2-image-2.0-0 \
		libsdl2-net-2.0-0 \
		libsdl2-ttf-2.0-0 \
		zlib1g \
	&& apt-get -y clean \
	&& printf '[general]\ndrivers = null\n' > /etc/openal/alsoft.conf \
	;

ENV LD_LIBRARY_PATH /usr/lib/x86_64-linux-gnu
