#!/bin/bash
current_sec=$(date '+%s')
dst_sec=$(date --date='00:00 today' '+%s')
diff_sec=$(($current_sec - $dst_sec))
echo -n $diff_sec
