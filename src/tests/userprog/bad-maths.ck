# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_DIV0_FAULTS => 1, [<<'EOF']);
(bad-maths) begin
bad-maths: exit(-1)
EOF
pass;
