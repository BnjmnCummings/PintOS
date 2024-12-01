# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(write-bad-ptr) begin
(write-bad-ptr) open "sample.txt"
write-bad-ptr: exit(-1)
EOF
pass;
