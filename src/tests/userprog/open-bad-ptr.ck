# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(open-bad-ptr) begin
open-bad-ptr: exit(-1)
EOF
pass;
