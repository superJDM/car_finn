#！/bin/bash

if [ `whoami` != 'root' ]
then

	u=`whoami`

	if [ `pwd` != /home/$u/nginx ]
	then
		echo '当前路径不是脚本路径大,当前路径为：'
		pwd
		cd /home/$u/nginx 
		echo "已变换为脚本执行路径:"
		pwd
	fi
	sleep 3

while
	clear

do
	echo "             ========================================    "
	echo "             *    nginx配 置 自 动 化 脚 本    *    "
	echo "             ----------------------------------------    "
	echo "             *   1. 查 询 当 前 路径           *    "
	echo "             *   2. 安 装 环境依赖包           *    "
	echo "             *   3. nginx 系 统 配置           *    "
	echo "             *   4. 查询已登记IP端口           *    "
	echo "             *   5. 添加IP地址与端口           *    "
	echo "             *   6. 证   书  设   置           *    "
	echo "             *   7. 启   动   nginx            *    "
	echo "             *   8. 退   出  系   统           *    "
	echo "             ========================================    "   
	echo "             请您选择: "

#stty -echo
read select

case $select in
	1)
		echo "当前路径为："
		pwd
		sleep 2
		continue
		;; 
	2)
		sudo rpm -ivh `find ./ -name kernel-headers-2.6.18-274.el5.x86_64.rpm`
		sudo rpm -ivh `find ./ -name glibc-headers-2.17-307.el7.1.x86_64.rpm`
		sudo rpm -ivh `find ./ -name glibc-devel-2.17-307.el7.1.x86_64.rpm`
		sudo rpm -ivh `find ./ -name mpfr-3.1.1-4.el7.x86_64.rpm`
		sudo rpm -ivh `find ./ -name libmpc-1.0.1-3.el7.x86_64.rpm`
		sudo rpm -ivh `find ./ -name cpp-4.8.5-39.el7.x86_64.rpm`
		sudo rpm -ivh `find ./ -name gcc-4.8.5-39.el7.x86_64.rpm`

		sudo rpm -ivh `find ./ -name libcom_err-devel-1.42.9-17.el7.x86_64.rpm` #(需要更新kernel-toolsc)
		sudo rpm -ivh `find ./ -name keyutils-libs-devel-1.5.8-3.el7.x86_64.rpm`
		sudo rpm -ivh `find ./ -name libkadm5-1.15.1-46.el7.x86_64.rpm`
		sudo rpm -ivh `find ./ -name pcre-devel-8.32-17.el7.x86_64.rpm`
	 	sudo rpm -ivh `find ./ -name libverto-devel-0.2.5-4.el7.x86_64.rpm`
		sudo rpm -ivh `find ./ -name zlib-devel-1.2.7-18.el7.x86_64.rpm`
		sudo rpm -ivh `find ./ -name libsepol-devel-2.5-10.el7.x86_64.rpm`
		sudo rpm -ivh `find ./ -name libselinux-devel-2.5-15.el7.x86_64.rpm` 
		sudo rpm -ivh `find ./ -name krb5-devel-1.15.1-46.el7.x86_64.rpm` 
		sudo rpm -ivh `find ./ -name openssl-devel-1.0.2k-19.el7.x86_64.rpm`
		echo "所有依赖包安装完毕"
		sleep 2
		continue
		;;
	3)

		#setup for nginx(with nginx already install)
		if [ `pwd` != /home/$u/nginx/nginx-1.14.0 ]
		then
		cd /home/$u/nginx/nginx-1.14.0
		fi
		./configure --prefix=/usr/local/nginx  --with-http_stub_status_module --with-http_ssl_module --with-file-aio --with-http_realip_module 
		make
		make install
		echo "nginx配置完成"
		sleep 2
		continue
		;;

	4)
		#screch upstream https{ip}
		if [ `pwd` != /home/$u/nginx/conf ]
		then
		cd /home/$u/nginx/conf/
		fi
		cat ./nginx.conf|grep -Pzo "upstream https{(?s).*?}"
		sleep 3
		continue
		;;
	5)
		#change nginx.config
		if [ `pwd` != /home/$u/nginx/conf ]
		then
		cd /home/$u/nginx/conf/
		fi
		#read
		echo "请输入web ip:"
		read x
		echo "请输入端口号:"
		read y
		echo "请输入开放api ip:"
		read m
		echo "请输入开放api端口号:"
		read n

		#add
		sudo sed "/upstream https{/a\        server $x:$y;" ./nginx.conf
		sudo sed "/upstream https{/a\        server $m:$n;" ./nginx.conf
		#repalce
		#sed -i "s/localhost:80/$x:$y/g" ./nginx.conf
		#sed -i "s/localhost:443/$m:$n/g" ./nginx.conf
		echo “添加完成！”
		sleep 2
		continue
		;;
	6)
		#renew certificate
		echo "lociate certificate path:"
		read path
		echo "证书设置完成"
		sleep 2
		continue
		;;
	7)
		#run ngnix
		/usr/local/nginx/sbin/nginx

		#check 
		nginx -t && echo "nginx 已启动"
		sleep 2
		continue
		;;

	8)
		exit
		;;
	*)	
		echo "请选择正确的数字，请重新选择"
		sleep 2
esac
done
else
	echo "系统管理员权限"
fi
