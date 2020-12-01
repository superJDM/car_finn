#! /bin/bash

#replace path to linux
sed -i 's/\\/\//g' ./configure/configure.txt

declare -a modi
declare -a file
declare key
declare -a value
declare -a used

# change IFS 
IFS=$'\n'

for line in `cat  ./configure/configure.txt`;
do 
	#echo $line
	#line=${line/$}
	if [ ${line:0:1} == '#' ];
	then
		if [ -f "${line:1}" ];then
			file[${#file[*]}]=${line:1}
		elif [ "$line" = "${line/\//}" ];then
			usleep
		else
			echo "请确认文件${line：1}路径或文件格式是否正确"
		fi
	elif [ "$line" = "${line/=/}" ];then
		usleep 1
	else
	 	modi[${#modi[*]}]=${line:0}
	fi	
    
done


#method 1 

for m in ${modi[*]};do
	#replace special char
	m=${m//\//\\/}
	m=${m//&/\\&}
	value[${#value[*]}]=${m#*=}
	key[${#key[*]}]=${m%%=*}
done

for f in ${file[*]};do
	if [ ${f##*.} == 'properties' ];then
		#list star from 0
		len=`expr ${#key[*]} - 1`
		#key_used=(${key[*]})
		for k in `seq 0 $len`;do
			sed -i "s/^${key[$k]}=.*$/${key[$k]}=${value[$k]}/" $f 
			# delete from key (done)
			grep -q "${key[$k]}" $f && used[${#used[*]}]=$k
			#key_used=(${key_used[*]/${key[$k]}/})		
	 		
		done
	else
		#len_used=`expr ${#key_used[*]} - 1`
		for k1 in `seq 0 $len`;do
		# notice input form
		#count1=0
			if [[ ${used[*]/$k1/} == ${used[*]} ]];then
					#spare key to two part by .
					kp1=${key[$k1]%%.*}
					kp2=${key[$k1]##*.}
					#long=`expr ${#kp2} - 1`
					#kp2=${kp2:0:$long} 
					sed -i "/${kp1}/,/${sp2}/s/${kp2}:.*/${kp2}: ${value[$k1]}/g" $f  
					#count1=`expr $count1 + 1`
			fi
		done
	fi
	if [ $? ];then 
		echo "文件${f##*/}已修改完成"
done

