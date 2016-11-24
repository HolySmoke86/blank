FROM localhorsttv/debian-run:testing

RUN apt -y install build-essential \
	&& apt-get -y clean \
	;

RUN apt -y install --no-install-recommends \
	cppcheck \
	libalut-dev \
	libcppunit-dev \
	libglew-dev \
	libglm-dev \
	libopenal-dev \
	libsdl2-dev \
	libsdl2-image-dev \
	libsdl2-net-dev \
	libsdl2-ttf-dev \
	zlib1g-dev \
	&& apt-get -y clean \
	;
