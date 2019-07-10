# IDP

IsarPipeline is a tool combining state-of-the-art methods to quickly generate extensive protein sequence alignments.

# Prerequisites
## Hardware requirements
:exclamation: Before you start compiling any code, make sure you have a machine that supports SSE4.1 or AVX2. You can check that by typing the following commands

cat /proc/cpuinfo | grep -c sse4_1 > /dev/null && echo "SSE4.1 Supported" || echo "SSE4.1 Unsupported"

OR

cat /proc/cpuinfo | grep -c avx2 > /dev/null && echo "AVX2 Supported" || echo "AVX2 Unsupported"

Once the hardware requirement is present, you can move to the software requirements.

## Software requirements
 :exclamation: Compiling the following software will require git, g++ (7.3 or higher) and cmake (3.0 or higher).
