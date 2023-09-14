CC = gcc

INCLUDE = -I include
CFLAGS += $(INCLUDE)
CFLAGS += -std=c11 -Wall -Wextra -pedantic \
		-Wconversion -Wwrite-strings -Wcast-align -Wpointer-arith \
		-Winit-self -Wshadow -Wstrict-prototypes -Wmissing-prototypes \
		-Wredundant-decls -Wundef -Wvla -Wdeclaration-after-statement
LDFLAGS += -lm -lcyaml -lpthread -L/usr/local/lib -lwiringPi

TARGET = rpi-pwm-fan-control
BUILD_DIR = build
INSTALL_DIR = /usr/local/bin
CONFIG_DIR = /usr/local/etc/$(TARGET)

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): main.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

install: $(BUILD_DIR)/$(TARGET)
	mkdir -p $(CONFIG_DIR)
	cp $< $(INSTALL_DIR)
	cp -n example/config.yaml $(CONFIG_DIR)
	cp example/unit.service /etc/systemd/system/$(TARGET).service

uninstall:
	rm -rf $(INSTALL_DIR)/$(TARGET)
	rm -rf $(CONFIG_DIR)
	rm /etc/systemd/system/$(TARGET).service

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all install clean
