# travis core分析
matrix.include 增加 
```
sudo:true
```
apt.packages 增加
```
gdb
```
script 增加
```
ulimit -c unlimited -S
sudo bash -c "echo '/tmp/core.%p.%E' > /proc/sys/kernel/core_pattern"
```
after_script 增加
```
ls -l /tmp
for i in $(find /tmp -maxdepth 1 -name 'core*' -print); do gdb XXX $i -ex "thread apply all bt" -ex "set pagination 0" -batch; done;
```