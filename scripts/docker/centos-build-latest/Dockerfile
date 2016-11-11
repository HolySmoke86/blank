FROM centos:latest

RUN yum -y update \
	&& yum -y install epel-release \
	&& yum -y update \
	&& yum -y group install "Development Tools"
RUN yum -y install \
	cppcheck \
	cppunit-devel \
	freealut-devel \
	glew-devel \
	glm-devel \
	openal-soft-devel \
	SDL2-devel \
	SDL2_image-devel \
	SDL2_net-devel \
	SDL2_ttf-devel \
	zlib-devel \
	;
