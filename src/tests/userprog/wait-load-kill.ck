# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF',<<'EOF',<<'EOF']);
(wait-load-kill) begin
(wait-load-kill) exec() returned a valid pid
(child-bad) begin
child-bad: exit(-1)
(wait-load-kill) wait() = -1
(wait-load-kill) end
wait-load-kill: exit(0)
EOF
(wait-load-kill) begin
(child-bad) begin
(wait-load-kill) exec() returned a valid pid
child-bad: exit(-1)
(wait-load-kill) wait() = -1
(wait-load-kill) end
wait-load-kill: exit(0)
EOF
(wait-load-kill) begin
(child-bad) begin
child-bad: exit(-1)
(wait-load-kill) exec() returned a valid pid
(wait-load-kill) wait() = -1
(wait-load-kill) end
wait-load-kill: exit(0)
EOF
pass;
