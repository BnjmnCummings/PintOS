# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(pt-overflowstk) begin
(pt-overflowstk) grew stack 1 times
(pt-overflowstk) grew stack 2 times
(pt-overflowstk) grew stack 3 times
(pt-overflowstk) grew stack 4 times
(pt-overflowstk) grew stack 5 times
(pt-overflowstk) grew stack 6 times
(pt-overflowstk) grew stack 7 times
(pt-overflowstk) grew stack 8 times
(pt-overflowstk) grew stack 9 times
(pt-overflowstk) grew stack 10 times
(pt-overflowstk) grew stack 11 times
(pt-overflowstk) grew stack 12 times
(pt-overflowstk) grew stack 13 times
(pt-overflowstk) grew stack 14 times
(pt-overflowstk) grew stack 15 times
(pt-overflowstk) grew stack 16 times
(pt-overflowstk) grew stack 17 times
(pt-overflowstk) grew stack 18 times
(pt-overflowstk) grew stack 19 times
(pt-overflowstk) grew stack 20 times
(pt-overflowstk) grew stack 21 times
(pt-overflowstk) grew stack 22 times
(pt-overflowstk) grew stack 23 times
(pt-overflowstk) grew stack 24 times
(pt-overflowstk) grew stack 25 times
(pt-overflowstk) grew stack 26 times
(pt-overflowstk) grew stack 27 times
(pt-overflowstk) grew stack 28 times
(pt-overflowstk) grew stack 29 times
(pt-overflowstk) grew stack 30 times
pt-overflowstk: exit(-1)
EOF
pass;
