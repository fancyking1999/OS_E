wget git.io/superupdate.sh && \
bash superupdate.sh aliyun && \
apt update && \
wget https://mirrors.aliyun.com/linux-kernel/v5.x/linux-5.0.tar.gz && \
cp ./linux-5.0.tar.gz /usr/src/ && \
cd /usr/src/ && \
tar zxvf linux-5.0.tar.gz && \
apt install libc6-dev \
			make \
			dpkg-dev \
			linux-libc-dev \
			libssl-dev kernel-package \
			libncurses5-dev \
			fakeroot \
			bc xz-utils \
			ca-certificates \
			ncurses-dev \
			libncurses5-dev \
			bison \
			flex \
			libelf-dev -y --fix-missing && \
apt upgrade -y && \
apt autoclean && \
apt clean && \
apt autoremove
