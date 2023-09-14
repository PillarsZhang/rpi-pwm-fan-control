# RPI PWM Fan Control

A lightweight Raspberry Pi PWM fan temperature control program written in C language. 
It can read YAML formatted configuration and run in the background as a systemd service. 
(Of course, you will need a hardware module, which can be homemade or purchased. Thanks to Taobao: 七彩智能科技.)

## Dependency

```bash
sudo apt install libyaml-dev
```

```bash
git clone https://github.com/tlsa/libcyaml.git
cd ./libcyaml
make install VARIANT=release
```

```bash
git clone https://github.com/WiringPi/WiringPi.git
cd ./WiringPi
./build
```

## Install

```bash
sudo make install
sudo systemctl enable rpi-pwm-fan-control.service
sudo systemctl start rpi-pwm-fan-control.service
```

## Config

The configuration file will be installed to `/usr/local/etc/rpi-pwm-fan-control/config.yaml`.

```yaml
channel: 15
temp_fan_start: 60
temp_fan_max: 75
temp_fan_stop: 55
pwm_start: 70
pwm_max: 100
delay: 2000
```

## Status

```bash
sudo systemctl status rpi-pwm-fan-control.service
sudo journalctl -u rpi-pwm-fan-control.service -f
```

## Uninstall

```bash
sudo systemctl disable rpi-pwm-fan-control.service
sudo systemctl stop rpi-pwm-fan-control.service
sudo make uninstall
```

## Update

The configuration file will not be overwritten.

```bash
sudo systemctl stop rpi-pwm-fan-control.service
sudo make install
sudo systemctl daemon-reload
sudo systemctl start rpi-pwm-fan-control.service
```