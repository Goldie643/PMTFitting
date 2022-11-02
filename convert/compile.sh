export BOOST=/opt/homebrew/Cellar/boost/1.80.0/include

g++  -std=c++14 -O2 -Wall -fPIC -pthread -m64 -I$BOOST -I$ROOTSYS/include -I$ROOTSYS/include/root -c main.cpp 
#g++  -O2 -m64 -std=c++11 main.o -lm -L$ROOTSYS/lib -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -pthread -lm -ldl -rdynamic -o waveconvert
g++  -O2 -m64 -std=c++14 main.o -lm $(root-config --libs) -rdynamic -o waveconvert
rm main.o
