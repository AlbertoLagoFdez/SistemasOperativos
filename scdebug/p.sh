#!./ptbash
echo "traced_pepe" > /proc/$$/comm
kill -SIGSTOP $$
#exec sleep 30
exec ls