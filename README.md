# FAT32
````
An emulation of FAT File System
````

## Compile and execute

### Compile
````
> make
````
This will produce bin/ directory of object files located within src/

Within Makefile, TARGET variable will be used for the name for the executable, this can be changed.

### Execute
````
> ./$(TARGET)
````

### Clean
````
> make clean
````
This will remove the bin/ dir holding object files related to src/*.cpp. Along with $(TARGET).exe