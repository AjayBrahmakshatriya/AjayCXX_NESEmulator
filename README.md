# NES Emulator with BuildIt

## Building
1. Make sure to clone with the --recursive flag to also clone the dependency - 

	git clone --recursive https://github.com/AjayBrahmakshatriya/AjayCXX_NESEmulator.git

2. First build the dependency (inside the project repo) - 

	make -C buildit 

3. Now build the project
	
	make 

## Running
Currently, the project just generates code for the program written in the driver.cpp. 

1. Edit the driver.cpp to feed in your MOS 6502 program
2. Run `./build/nes_compiler`

The generated code for the program will be printed




