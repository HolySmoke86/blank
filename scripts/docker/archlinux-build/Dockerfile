FROM localhorsttv/archlinux-run:latest

RUN pacman -S --needed --noconfirm \
	base-devel \
	cppcheck \
	cppunit \
	git \
	glm \
	&& \
	paccache -rk 0
