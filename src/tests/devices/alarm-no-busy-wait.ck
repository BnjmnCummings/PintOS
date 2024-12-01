# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(alarm-no-busy-wait) begin
(alarm-no-busy-wait) Creating 5 threads to sleep 1 times each.
(alarm-no-busy-wait) Each thread sleeps for 20 ticks at a time,
(alarm-no-busy-wait) Test is successful if the threads are not running
(alarm-no-busy-wait) when they are supposed to be asleep.
(alarm-no-busy-wait) 0 threads on the ready list
(alarm-no-busy-wait) PASS
(alarm-no-busy-wait) end
EOF
pass;
