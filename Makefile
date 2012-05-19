
default: xenon

all: release

xenon:
	$(MAKE) -C source PLATFORM=xenon BUILD=xenon_debug
clean: 
	$(MAKE) -C source clean

install: xenon
	$(MAKE) -C source install

