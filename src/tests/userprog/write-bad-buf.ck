# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_KERNEL_FAULTS => 1, [<<'EOF']);
(write-bad-buf) begin
(write-bad-buf) open "sample.txt"
write-bad-buf: exit(-1)
EOF
pass;