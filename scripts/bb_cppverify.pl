#!/usr/bin/env perl
# bb_cppverify.pl                                                    -*-perl-*-

# Bb_cppverify is a static analysis tool that verifies that source code adheres
# to the BDE coding standards.  This version of the wrapper script is for
# Microsoft Windows.

use strict;
use warnings;
use Getopt::Long qw(:config no_ignore_case);
use Cwd;
use File::Basename;
use File::Glob ':bsd_glob';

my $bb       = $ENV{BDE_ROOT} || '';
$bb          = '' unless -d $bb;

my $me       = Cwd::abs_path(${0});
my $pt       = $me;
$pt          =~ s{(bin/|scripts/)?[^/]*$}{};
my $nm       = ${0};
$nm          =~ s{.*[/\\]}{};
$nm          =~ s{[.]pl}{};

my $config   = "${pt}etc/bde-verify/${nm}.cfg";
$config      = "${pt}${nm}.cfg"                    unless -r $config;

my $enm      = "bde_verify_bin";
my $exe      = "${pt}libexec/bde-verify/$enm.exe";
$exe         = "${pt}$enm.exe"                     unless -x $exe;
$exe         = "$enm.exe"                          unless -x $exe;

my @cl;
my $debug    = "";
my $verbose  = "";
my $version  = "";
my $help     = "";
my $definc   = 1;
my $defdef   = 1;
my @incs;
my @defs;
my @wflags;
my @lflags = (
    "cxx-exceptions",
    "exceptions",
    "diagnostics-show-note-include-stack",
    "diagnostics-show-option",
    "error-limit=0",
    "ms-compatibility",
    "ms-compatibility-version=19",
    "ms-extensions",
   #"msc-version=1800",
   #"delayed-template-parsing",
);
my $dummy;
my @dummy;
my $std;
my @std;
my $warnoff;
my $diagnose = "component";
my $rwd;
my $rwf;
my $tag;
my $ovr = -1;
my $m32;
my $m64;
my $diff = "";

my $command = join " \\\n ", $0, map { join '" "', split(/ /, $_, -1) } @ARGV;

sub usage()
{
    print "
usage: $0 [options] [additional compiler options] file.cpp ...
    -I{directory}
    -D{macro}
    -w                       # disable normal compiler warnings
    -W{warning}
    -f{flag}
    --config=config_file     [$config]  
    --cl=config_line
    --bb=dir                 [$bb]
    --exe=binary             [$exe]
    --diff=file (- = stdin)  [$diff]
    --[no]definc             [$definc]
    --[no]defdef             [$defdef]
    --[no]ovr                # whether to define BSL_OVERRIDES_STD
    --rewrite-dir=dir
    --rewrite-file=file
    --diagnose={main,component,nogen,all}
    --std=type
    --tag=string
    --debug
    --verbose
    --version
    --help

For full documentation, see {TEAM BDE:BB_CPPVERIFY<GO>} or
<http://cms.prod.bloomberg.com/team/display/bde/bb_cppverify>.

Invoked as:
$command
";
    exit(1);
}

@ARGV = map { /^(-+[DIOWf])(.+)/ ? ($1, $2) : $_ } @ARGV;

GetOptions(
    'bb=s'                         => \$bb,
    'config=s'                     => \$config,
    'cl=s'                         => \@cl,
    'debug'                        => \$debug,
    'help|?'                       => \$help,
    'cc=s'                         => \$dummy,
    'diff=s'                       => \$diff,
    'exe=s'                        => \$exe,
    'verbose|v'                    => \$verbose,
    'version'                      => \$version,
    'definc!'                      => \$definc,
    'defdef!'                      => \$defdef,
    'ovr!'                         => \$ovr,
    'nsa!'                         => \$dummy,
    'rewrite|rewrite-dir|rd=s'     => \$rwd,
    'rewrite-file|rf=s'            => \$rwf,
    'w'                            => \$warnoff,
    "I=s"                          => \@incs,
    "D=s"                          => \@defs,
    "W=s"                          => \@wflags,
    "f=s"                          => \@lflags,
    "std=s"                        => \$std,
    "toplevel"                     => sub { $diagnose = "main" },
    "diagnose=s"                   => sub {
        my %opts = ("main" => 1, "component" => 1, "nogen" => 1, "all" => 1);
        die "Unknown value '$_[1]' for '-$_[0]='\n" unless $opts{$_[1]};
        $diagnose = $_[1];
    },
    'tag=s'                        => \$tag,
    'llvm=s'                       => \$dummy,
    "m32"                          => \$m32,
    "m64"                          => \$m64,
    "pipe|pthread|MMD|g|c|S"       => \$dummy,
    "O|MF|o|march|mtune=s"         => \@dummy,
) and !$help and ($#ARGV >= 0 or $version) or usage();

sub xclang(@) { return map { ( "-Xclang", $_ ) } @_; }
sub plugin(@) { return xclang( "-plugin-arg-bde_verify", @_ ); }

my @config = plugin("config=$config")      if $config ne "";
my @debug  = plugin("debug-on")            if $debug;
my @tlo    = plugin("diagnose=$diagnose");
my @rwd    = plugin("rewrite-dir=$rwd")    if $rwd;
print "No such directory $rwd\n" if $rwd and ! -d $rwd;
my @rwf    = plugin("rewrite-file=$rwf")   if $rwf;
my @tag    = plugin("tool=$tag")           if $tag;
my @diff   = plugin("diff=$diff")          if $diff;
my %uf     = (
             );
@lflags = map { "-f$_" } grep { not exists $uf{$_} } @lflags;
@wflags = map { "-W$_" } grep { not /,/ } @wflags;
unshift @wflags, "-w" if $warnoff;

my $macro = ${nm};
$macro =~ s{^([^.]*).*}{\U$1\E};
push(@defs, ($macro));
push(@defs, (
    "BB_THREADED",
    "BDE_BUILD_TARGET_DBG",
    "BDE_BUILD_TARGET_EXC",
    "BDE_BUILD_TARGET_MT",
    "BDE_DCL=",
    "BDE_OMIT_INTERNAL_DEPRECATED",
    "BSLS_IDENT_OFF",
    "BSL_DCL=",
    "DEBUG",
    "NOGDI",
    "NOMINMAX",
    "WINVER=0x0500",
    "_CRT_SECURE_NO_DEPRECATE",
    "_MT",
    "_SCL_SECURE_NO_DEPRECATE",
    "_STLP_HAS_NATIVE_FLOAT_ABS",
    "_STLP_USE_STATIC_LIB",
    "_WIN32_WINNT=0x0500",
)) if $defdef;
if ($ovr == 1 || $ovr != 0 && $defdef && !grep(m{\bb[sd]l[^/]*$}, @ARGV)) {
    push(@defs, "BSL_OVERRIDES_STD");
}
@defs = map { "-D$_" } @defs;
if ($ovr == 0) {
    push(@defs, "-UBSL_OVERRIDES_STD");
}

for (@ARGV) {
    if (! -f) {
        warn "Cannot find file $_\n";
    } elsif (m{^(.*)[/\\].*$}) {
        push(@incs, $1) unless grep($_ eq $1, @incs);
    } else {
        push(@incs, ".") unless grep($_ eq ".", @incs);
    }
}
for (@incs) { warn "cannot find directory $_\n" unless -d; }
push(@incs, (
    "$bb/include",
    "$bb/include/bsl+stdhdrs",
)) if $bb and $definc;
@incs = map { ( "-I", $_ ) }
        map { $_ eq Cwd::cwd() ? "." : $_ }
        map { Cwd::abs_path($_) }
        grep { -d } @incs;

my @pass;
push(@pass, "-m32") if $m32;
push(@pass, "-m64") if $m64;
push(@pass, "--version") if $version;

# Try to find include paths if INCLUDE is not specified.
my $inc = $ENV{INCLUDE};

if (!$inc) {
    sub get_dir
    {
        my $file = $_[0];
        my @dirs = @_;
        my @globs;
        for my $dir (@dirs) {
            push @globs, (
                "$dir/Include/*/$file",
                "$dir/Include/[1-9]*/*/$file",
                "$dir/VC/Tools/MSVC/*/atlmfc/include/$file",
                "$dir/VC/Tools/MSVC/*/include/$file",
                "$dir/VC/atlmfc/include/$file",
                "$dir/VC/include/$file",
                "$dir/*/*/VC/Tools/MSVC/*/atlmfc/include/$file",
                "$dir/*/*/VC/Tools/MSVC/*/include/$file",
            );
        }
        my $found;
        for my $glob (@globs) {
            for my $file (bsd_glob($glob, GLOB_NOCASE)) {
                $found = $file if !$found || -M $file < -M $found;
            }
        }
        return Cwd::abs_path(dirname($found || './'));
    }

    my $p = $ENV{'ProgramFiles(x86)'} || 'C:/Program Files (x86)';
    my @vswhere = (
        "$p/Microsoft Visual Studio/Installer/vswhere.exe",
        '-property', 'installationpath', '-latest'
    );
    my @paths;
    if (-x $vswhere[0] && open(my $fh, "-|", @vswhere)) {
        while (<$fh>) {
            s/\r?\n$//;
            push @paths, $_;
        }
    }
    if ($#paths < 0) {
        @paths = bsd_glob("$p/Microsoft Visual Studio*", GLOB_NOCASE);
    }
    push @paths, bsd_glob("$p/Windows Kits/[1-9]*", GLOB_NOCASE);

    my @exemplars = (
        'cstdio',
        'atlbase.h',
        'windef.h',
        'wincon.h',
        'winstring.h',
    );
    my $sep = '';
    for my $exemplar (@exemplars) {
        $inc .= $sep . get_dir($exemplar, @paths);
        $sep = ';';
    }
}

push(@incs, map { ( "-isystem", $_ ) }
            map { Cwd::abs_path($_) }
            grep { -d } split(/;/, $inc));

push(@std, "-std=$std") if $std;

@cl = map { plugin("config-line=$_") } @cl;

my @command = (
    "$exe",
    xclang("-plugin", "bde_verify"),
    "-resource-dir", "${pt}include/bde-verify/clang",
    "-msoft-float",
    "-fsyntax-only",
    "-xc++",
    @std,
    @debug,
    @config,
    @tlo,
    @rwd,
    @rwf,
    @tag,
    @diff,
    @cl,
    @defs,
    @incs,
    @lflags,
    @wflags,
    @ARGV,
);

map { s{\\}{/}g } @command;

if ($verbose)
{
    my @c = @command;
    map { s{(.*)}{"$1"} if m{[( )]} and not m{"} } @c;
    print join(" \\\n ", @c), "\n";
}

exec @command;

## ----------------------------------------------------------------------------
## Copyright (C) 2014 Bloomberg Finance L.P.
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
