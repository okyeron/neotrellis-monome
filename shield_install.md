# recompiling norns to allow neotrellis grid to be recognized on Norns or Norns Shield

NOTE - Be aware this is a hack/workaround and is not officially supported.
Proceed at your own risk

This fix will need to be re-applied after any norns SYSTEM>UPDATE

```
cd ~/
sudo apt-get update
sudo apt-get install libncurses5-dev libncursesw5-dev
wget https://raw.githubusercontent.com/okyeron/fates/master/install/norns/files/device/device_monitor.c
cd ~/norns
git pull
git submodule update --init --recursive
sudo cp -f /home/we/device_monitor.c /home/we/norns/matron/src/device/device_monitor.c
rm /home/we/device_monitor.c
./waf clean
./waf configure --enable-ableton-link
./waf build
```

then reboot
