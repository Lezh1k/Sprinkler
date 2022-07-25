#!/bin/bash
current_sec=$(date '+%s')
dst_sec=$(date --date='00:00 tomorrow' '+%s')
diff_sec=$(($dst_sec - $current_sec))
echo $diff_sec
