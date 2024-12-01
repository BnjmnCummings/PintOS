# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF',<<'EOF']);
(sc-bad-num) begin
sc-bad-num: exit(-1)
EOF
(sc-bad-num) begin
(sc-bad-num) invalid syscall did not terminate the process, so check its return value...
(sc-bad-num) end
sc-bad-num: exit(0)
EOF
pass;
