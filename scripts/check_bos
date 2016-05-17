#!/usr/bin/env perl
# check_bos                                                          -*-perl-*-
#
# This is a wrapper script around bde_verify which checks that code is neutral
# with respect to BSL_OVERRIDES_STD.  It exits with a non-zero status if not.

use strict;
use warnings;

use Getopt::Long qw(:config no_ignore_case pass_through);

my $bv = 'bde_verify';
my $help;

my $command = join " \\\n ", $0, map { join "\\ ", split(/ /, $_, -1) } @ARGV;

sub usage()
{
    print "
usage: $0 [options] [bde_verify arguments]...
    -bv=bde_verify      path to bde_verify [$bv]
    -help               print this message

Invoked as:
$command
";
    exit(1);
}

GetOptions(
    'bv=s'   => \$bv,
    'help|?' => \$help,
) and !$help and $#ARGV >= 0 or usage();

my @args = (
    '-diagnose=main',
    '-cl=all off',
    '-cl=check bsl-overrides-std on',
    '-cl=suppress IS01 *',
    '-cl=suppress IS02 *',
    '-cl=suppress SB01 *',
    '-cl=suppress SB02 *',
    '-cl=suppress SB03 *',
    '-cl=set failstatus SB04',
);

exit 1 if system $bv, @ARGV, @args, '-ovr';
exit 1 if system $bv, @ARGV, @args, '-noovr';

## ----------------------------------------------------------------------------
## Copyright (C) 2015 Bloomberg Finance L.P.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
## ----------------------------- END-OF-FILE ----------------------------------
