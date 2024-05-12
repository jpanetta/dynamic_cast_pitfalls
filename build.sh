# Create a menu to choose between build modes (if the choice is not passed as an argument)
if [ $# -eq 1 ]; then
    choice=$1
else
    echo "Choose build mode:"
    echo "1) Out-of-line classes with EXPORT_CLASS (correct)"
    echo "2) Inline classes without EXPORT_CLASS (dynamic_cast error 2 printed to syslog)"
    echo "3) Inline classes with    EXPORT_CLASS (OK unless EXPORT_CLASS is forgotten one one derived class)"
    echo "4) Static libderived with out-of-line exported classes (dynamic cast failures despite matching typinfo names)"
    echo "5) Static libderived with inline, exported classes (OK)"

    read -p "Enter choice: " choice
fi

case $choice in
    1)
        CONFIG_STRING="-DOUT_OF_LINE_DTOR -DDO_EXPORT_CLASS"
        DYNAMIC_LIBDERIVED=1
        ;;
    2)
        CONFIG_STRING="-UOUT_OF_LINE_DTOR -UDO_EXPORT_CLASS"
        DYNAMIC_LIBDERIVED=1
        ;;
    3)
        CONFIG_STRING="-UOUT_OF_LINE_DTOR -DDO_EXPORT_CLASS"
        DYNAMIC_LIBDERIVED=1
        ;;
    4)
        CONFIG_STRING="-DOUT_OF_LINE_DTOR -DDO_EXPORT_CLASS"
        DYNAMIC_LIBDERIVED=0
        ;;
    5)
        CONFIG_STRING="-UOUT_OF_LINE_DTOR -DDO_EXPORT_CLASS"
        DYNAMIC_LIBDERIVED=0
        ;;
    *)
        echo "Invalid choice"
        exit 1
        ;;
esac

CFLAGS="-O0 -g -fvisibility=hidden -std=c++14 -fPIC $CONFIG_STRING"
LIBFLAGS="-shared -fvisibility=hidden"

rm -rf build
mkdir build

g++ $CFLAGS -c lib1.cc -o build/lib1.o
g++ $CFLAGS -c lib2.cc -o build/lib2.o
g++ $CFLAGS -c Base.cc -o build/libbase.o

g++ $CFLAGS -c Derived.cc -o build/derived.o

g++ $CONFIG_STRING -O0 -g -std=c++14 -c main.cc -o build/main.o

g++ $LIBFLAGS -o build/libbase.so build/libbase.o

if [ $DYNAMIC_LIBDERIVED -eq 1 ]; then
    g++ $LIBFLAGS -o build/libderived.so build/derived.o -Lbuild -lbase
else
    ar rcs build/libderived.a build/derived.o
fi

g++ $LIBFLAGS -o build/lib1.so build/lib1.o -Lbuild -lbase -lderived
g++ $LIBFLAGS -o build/lib2.so build/lib2.o -Lbuild -lbase -lderived
g++ -o main build/main.o -Lbuild -l1 -l2 -lbase -lderived
