sudo apt-get -qq update
sudo apt-get -qq --yes --force-yes install libgtest-dev autoconf automake libtool libgoogle-glog-dev libgflags-dev
cd /usr/src/gtest
sudo mkdir build && cd build
sudo cmake ..
sudo make
sudo cp *.a /usr/lib
cd && git clone https://github.com/beniz/libcmaes.git
cd libcmaes
./autogen.sh
./configure
make
sudo make install
sudo ldconfig
cd $CI_HOME
