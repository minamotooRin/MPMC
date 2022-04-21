if [ -d "build" ]; then
  rm -rf build
fi
mkdir build
cd build

echo "Now start compile."
cmake ..
if [ $? != 0 ]
then
    echo "Compile Error!"
    exit 1
fi
make
if [ $? != 0 ]
then
    echo "Compile Error!"
    exit 1
fi
cd ..

echo "Compile done."