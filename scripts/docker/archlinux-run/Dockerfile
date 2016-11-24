FROM base/archlinux

RUN echo 'Server = http://mirror.23media.de/archlinux/$repo/os/$arch' > /etc/pacman.d/mirrorlist \
	&& pacman-key --init \
	&& pacman-key --populate archlinux \
	&& pacman -Sy --noconfirm archlinux-keyring \
	&& pacman -S --noconfirm pacman \
	&& pacman-db-upgrade \
	&& pacman -Su --noconfirm \
	&& pacman -S --noconfirm \
		git \
		make \
		mesa-libgl \
		xorg-server-xvfb \
	&& paccache -rk 0 \
	;

RUN pacman -S --needed --noconfirm \
		freealut \
		glew \
		openal \
		sdl2 \
		sdl2_image \
		sdl2_net \
		sdl2_ttf \
		zlib \
	&& paccache -rk 0 \
	&& mkdir -p /etc/openal \
	&& echo -e '[general]\ndrivers = null' > /etc/openal/alsoft.conf \
	;
