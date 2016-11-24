FROM localhorsttv/centos-run:latest

RUN yum -y group install "Development Tools" \
	&& yum -y install \
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
	&& yum -y clean packages \
	;
