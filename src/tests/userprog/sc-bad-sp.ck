# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_KERNEL_FAULTS => 1, [<<'EOF']);
(sc-bad-sp) begin
sc-bad-sp: exit(-1)
EOF
pass;
