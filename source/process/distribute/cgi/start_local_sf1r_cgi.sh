for i in $(seq 1 10); do
    spawn-fcgi -s "/tmp/fcgi/sf1r$i.socket" -f "./sf1r_process.fcgi 127.0.0.1:18181 10.10.1.112 8765 nginx.log.172.16.0.168 >> /tmp/fcgi/sf1r$i.log 2>&1" -P ./sf1r_process.fcgi.pid -F 1
done
