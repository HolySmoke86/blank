FROM centos:latest

RUN yum -y update \
	&& yum -y install epel-release \
	&& yum -y update \
	&& yum -y install \
		make \
		which \
		xorg-x11-server-utils \
		xorg-x11-server-Xvfb \
	&& yum -y clean packages \
	;

RUN yum -y install \
	freealut \
	glew \
	openal-soft \
	SDL2 \
	SDL2_image \
	SDL2_net \
	SDL2_ttf \
	zlib \
	&& yum -y clean packages \
	&& echo -e '[general]\ndrivers = null' > /etc/openal/alsoft.conf \
	;
