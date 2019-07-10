# IsarPipeline

IsarPipeline is a tool combining state-of-the-art methods to quickly generate extensive protein sequence alignments.

# Prerequisites
## Hardware requirements
:exclamation: Before you start compiling any code, make sure you have a machine that supports SSE4.1 or AVX2. You can check that by typing the following commands

        cat /proc/cpuinfo | grep -c sse4_1 > /dev/null && echo "SSE4.1 Supported" || echo "SSE4.1 Unsupported"

OR

        cat /proc/cpuinfo | grep -c avx2 > /dev/null && echo "AVX2 Supported" || echo "AVX2 Unsupported"

Once the hardware requirement is present, you can move to the software requirements.

## Software requirements
 :exclamation: Compiling the following software will require 'git', 'g++' (7.3 or higher) and 'cmake' (3.0 or higher).
 
### Install g++ 
The commands bellow are taillored to CentOS 7, a Linux distribution. You can lookup the equivalent command for each step. In most of the time, you just need to replace 'yum' by 'apt-get'. Refer to the support page bellow to figure out the equivalent in other Linux distributions:
        https://help.ubuntu.com/community/SwitchingToUbuntu/FromLinux/RedHatEnterpriseLinuxAndFedora

> You can either install gcc from repository by typing:

        yum -y install gcc

In order to include c++ library as well, you may want to install gcc-c++. This package will compile files with extensions that indicate they are C source as C++, instead of as C. You can install it by typing:

        yum -y install gcc-c++
        gcc --version

Depending on your version of your CentOS, you may see that GCC is not the last version available. In my machine, CentOS 7, the version distributed in this OS is GCC 4.8.5

Since we need GCC 7.3 or higher, we may need to install it from source. To do so, follow the next steps:

### Install MMseqs2
