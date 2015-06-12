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
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to
## deal in the Software without restriction, including without limitation the
## rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
## sell copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in
## all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
## FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
## IN THE SOFTWARE.
## ----------------------------- END-OF-FILE ----------------------------------
