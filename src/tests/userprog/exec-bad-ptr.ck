# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(exec-bad-ptr) begin
exec-bad-ptr: exit(-1)
EOF
pass;
