一些可能会用到的mysql操作
===

## 完全删除mysql数据库

光apt remove是删不掉的,数据库文件会保留下来.百度看到的一些博客上都一顿操作,似乎还不怎么正确.要完全删除,可以这样:

```bash
# 删除
sudo apt-get remove mysql-*
dpkg -l |grep ^rc|awk '{print $2}' |sudo xargs dpkg -P

# 重装
sudo apt-get install mysql-client mysql-server
```

## 允许外网访问


第一步:

```bash
#注意,要把`mypwd`修改为自己的密码!
mysql> GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' IDENTIFIED BY 'mypwd' WITH GRANT OPTION;
mysql> flush privileges;
```

第二步:

修改配置文件,把bind-address=127.0.0.1删掉

配置文件在这里:`/etc/mysql/mysql.conf.d/mysqld.cnf `
