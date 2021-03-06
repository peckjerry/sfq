sfq - Simple File-based Queue
===
sfq is a library that provides a file-based queue function in a multi-threaded / multi-process environment.

![how to use](http://sfq.iret.co.jp/sfq-how-to-use.png)

[Development Environment]
* OS) CentOS 6/7 64bit

[Build]
* 1) yum -y install jansson-devel libcap-devel libuuid-devel
* 2) git clone (get source tree)
* 3) make (use sfq/Makefile)

[Binaries]
* Library) sfq/lib/lib*.so
* Exe) sfq/bin/sfqc-*

[Set Path & DLL-Path]
* export LD_LIBRARY_PATH="** sfq-dir **/lib:${LD_LIBRARY_PATH}"
* export PATH="** sfq-dir **/bin:${PATH}"

[Run) Queue Type-1]

Queue is static. you can add data (by sfqc-pusht command) and you can get data (by sfqc-shift command)

for example
* 1) sfqc-init ("noname" directory is made in "/var/tmp")
* 2) sfqc-pusht -v aaa
* 3) sfqc-pusht -v bbb
* 4) sfqc-pusht -v ccc (added 3 record)
* 5) sfqc-list (print records)
* 6) sfqc-shift (you got "aaa")
* 7) sfqc-pop (you got "ccc")
* 8) sfqc-clear (clear all records)

[Run) Queue Type-2]

Queue is dynamic. you can add data (by command) but data is automatically retrieved

for example
* 1) rm -rf /var/tmp/noname (delete the queue that was made before)
* 2) sfqc-init -B 1
* 3) sfqc-pusht -v 'date > /tmp/aaa.txt' (you get to tell the current time in "/tmp/aaa.txt")
... text("date > /tmp/aaa.txt") is sent automatically to the standard input of "/bin/sh"

[Command Options]
...

## License
* MIT  
    * see MIT-LICENSE.txt

