# OpenRASP 单文件安装程序

`build.sh` 会构建一个 Java 单文件安装程序，同时包含批量部署脚本和 Java 安装包文件。

构建方法
1. 返回到源代码根目录，执行 `./build-java.sh` 构建 `rasp-java.tar.gz`
2. 确认日志上传服务器 URL，这里是 `http://packages.baidu-int.com:8066/collect.php`
3. 在本目录执行 `./build.sh http://packages.baidu-int.com:8066/collect.php` 构建 `installer.sh`

用法
1. 安装 ./installer.sh -i -a appid -b appsecret -c backendurl
2. 卸载 ./installer.sh -u
