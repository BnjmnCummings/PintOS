# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF',<<'EOF']);
(overflow-stack) begin
overflow-stack: exit(-1)
EOF
(overflow-stack) begin
(overflow-stack) buffer: aaaaaaaaaa
(overflow-stack) end
overflow-stack: exit(0)
EOF
pass;