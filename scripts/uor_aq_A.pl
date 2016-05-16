#!/bbs/opt/bin/csperl5.12

use strict;
use warnings;

my $path;

BEGIN {
    chomp($path = qx{dirname @{[qx{which uorinfo}]}});
}

use lib "$path/../../lib/site_perl";
use lib "$path/../../lib/perl";

use Symbol::Constants qw/ARCH_SOLARIS/;
use Symbol::DB::Scratch;
use Symbol::Package;
use Symbol::Snapshot;
use Symbol::Metadata;
use File::Basename;
use File::stat;
use Getopt::Long qw(:config no_ignore_case);

my %known_systems = ('AIX' => 1, 'Linux' => 1, 'SunOS' => 1);
chomp(my $system = `uname -s`);
$system = 'Linux' unless exists $known_systems{$system};

my %pathmap = (
    'AIX'   => 'AIX-powerpc-64',
    'Linux' => 'Linux-x86_64-64',
    'SunOS' => 'SunOS-sparc-64',
);
my $bb = "/bb/build/$pathmap{$system}/release/robolibs/trunk";
my @bbd = (
    "$bb/opt/bb/include",
    "$bb/opt/bb/include/00depbuild",
    "$bb/opt/bb/include/00deployed",
    "$bb/opt/bb/include/00offlonly",
    "/bb/build/share/stp/include/00offlonly",
    "/bbsrc/thirdparty/bbit/include",
    "$bb/dpkgroot/opt/bb/include",
    "$bb/dpkgroot/opt/bb/include/stlport",
);
my $help;
my $nr;
my $local;
my $verbose;

my (%distrib, %libmap_cache, $db, $sns, %pkgs);

my $cachefile = ".uor_include_cache";

my $command = join " \\\n ", $0, map { join "\\ ", split(/ /, $_, -1) } @ARGV;

my @bh = @bbd;
s{$bb}{\$bb} for @bh;

sub usage()
{
    print qq<
usage: $0 [options] uor...
    --bb=dir     [$bb]
                 Base directory for header file searches.
                 Searched directories are

                     @{[join("\n                     ", @bh)]}

    --cache=file [$cachefile]
                 Cache file for storing file names from searched directories.
                 This file will be refreshed if it does not exist or if any of
                 the directories have been modified since the cache file was
                 created.  Specify no name (--cache='') not to use a cache.

    --local      [$local]
                 Do not use the uor database.  Command line arguments are
                 treated as source directories.

    --norefresh  [$nr]
    --nr         Do not refresh cache file even if out of date.

    --verbose    [$verbose]
                 Operate loudly.

    --help       Print this summary.
    --?

Invoked as:
$command
>;
    exit(1);
}

GetOptions(
    'bb=s'         => \$bb,
    'cache=s'      => \$cachefile,
    'help|?'       => \$help,
    'norefresh|nr' => \$nr,
    'local!'       => \$local,
    'verbose!'     => \$verbose,
) and !$help and $#ARGV >= 0 or usage();

sub init_db {
    $db  //= Symbol::DB::Scratch->new;
    $sns //= Symbol::Snapshot::getSnapshotForBranch(
        $db, 'trunk+stp', ARCH_SOLARIS);
}

sub suss_group_by_dirs {
    my ( $uor, $dir ) = @_;
    return map { basename $_ } grep { -d $_ } glob("$dir/$uor*");
}

sub suss_group_by_mk {
    my ( $uor, $dir ) = @_;
    open(my $fh, "<", "$dir/$uor.mk") or return;
    local $/ = undef;
    my $line = <$fh>;
    close $fh;
    $line =~ s{\\\n}{}g;
    $line =~ s{.*^ *SRCDIRS *= *([^\n]*).*}{$1}ms;
    return split ' ', $line if $line;
}

sub suss_group_by_mem {
    my ( $uor, $dir ) = @_;
    open(my $fh, "<", "$dir/group/$uor.mem") or return;
    my @r;
    while (<$fh>) {
        chomp;
        push(@r, $_) if /^[[:alnum:]]/;
    }
    close($fh);
    return @r;
}

sub libmap_local {
    my ( $dir ) = @_;
    my %info;

    return $libmap_cache{$dir} if exists $libmap_cache{$dir};

    if (! -d $dir) {
        warn "No such directory $dir\n";
        return $libmap_cache{$dir};
    }

    my $type;
    if ($dir =~ m!/package/*$! && -d $dir) {
        $type = 'package';
        $dir = dirname($dir);
    } elsif ($dir =~ m!/group/*$!) {
        $type = 'group';
        $dir = dirname($dir);
    } elsif (-d "$dir/package") {
        $type = 'package';
    } elsif (-d "$dir/group") {
        $type = 'group';
    } else {
        warn "Cannot find package or group directory for $dir\n";
        return $libmap_cache{$dir};
    }
    %info = (
        symname => $dir,
        type    => $type,
        metadir => $dir,
        dir     => $dir,
    );
    return $libmap_cache{$dir} = \%info;
}

sub libmap {
    my ( $uor ) = @_;

    return $libmap_cache{$uor} if exists $libmap_cache{$uor};

    init_db();

    my $pkg = $pkgs{$uor} //=
      Symbol::Package::readFromDB( $db, $sns, { name => $uor } )
      or do { return; };

    return if $uor =~ /^(?:Bbit|System)$/;

    my @meta =
      Symbol::Metadata::readArrFromDB( $db, $sns, { pkgid => $pkg->getId } )
      or return $libmap_cache{$uor};

    my $dir = dirname($meta[0]->getFullFilename);
    my %info = (
        symname => dirname($dir),
        type => $dir =~ m!/package$! ? 'package' : 'group',
    );
    $info{metadir} = $dir // '';
    $info{dir}     = $info{symname};

    return $libmap_cache{$uor} = \%info;
}

sub init_uor_include_cache
{
    my $fh;
    my $cmd = qq{find @bbd -name '*_*.h'};
    if ($cachefile) {
        my $t = 0;
        for my $d (@bbd) {
            my $ds = stat($d);
            $t = $ds->mtime if $ds and $ds->mtime > $t;
        }
        my $cs = stat($cachefile);
        if (!$cs || $cs->mtime < $t && !$nr) {
            qx{$cmd > $cachefile};
        }
        open($fh, "<", $cachefile) or die "Cannot open $cachefile: $!\n";
    } else {
        open($fh, "-|", $cmd);
    }
    while (<$fh>) {
        chomp(my $f = $_);
        $f =~ s{.*/}{};
        my @s = split '_', $f;
        for my $i (0 .. $#s - 1) {
            my $p = join('_', @s[0 .. $i]);
            push(@{$distrib{$p}}, $f);
        }
    }
    close($fh);
}

sub check {
    init_uor_include_cache if not %distrib;

    my ($pre, @files) = @_;
    my %f;
    my $bad = 0;
    for my $f (@files) {
        my $s = $f;
        $s =~ s{.*/}{};
        $f{$s} = 1;
    }
    for my $h (@{$distrib{$pre}}) {
        if (!exists $f{$h}) {
            warn "Did not find '$h'.\n" if $verbose;
            print "$h\n";
            $bad = 1;
        } else {
            warn "Found '$h'.\n" if $verbose;
        }
    }
    return $bad;
}

my $bad = 0;
for my $arg (@ARGV) {
    my $result = $local ? libmap_local $arg : libmap $arg;
    if ( $result ) {
        my $dir = $result->{dir};
        $arg = basename($dir);
        if ( $result->{type} eq "package" ) {
            $bad = check($arg, glob "$dir/*.h") || $bad;
        } else {
            my @r3 = suss_group_by_dirs($arg, $dir);
            my @r1 = suss_group_by_mem($arg, $dir);
            for my $d (@r1) {
                warn "No directory '$dir/$d' for '$d' " .
                     "in '$dir/group/$arg.mem'\n"
                unless         grep { $_ eq $d } @r3;
            }
            for my $d (@r3) {
                warn "'$d' not in '$dir/group/$arg.mem' " .
                     "for directory '$dir/$d'\n"
                unless !@r1 or grep { $_ eq $d } @r1;
            }
            my @r2 = suss_group_by_mk($arg, $dir);
            for my $d (@r2) {
                warn "No directory '$dir/$d' for '$d' ".
                     "in SRCDIRS of '$dir/$arg.mk'\n"
                unless         grep { $_ eq $d } @r3;
            }
            for my $d (@r3) {
                warn "'$d' not in SRCDIRS of '$dir/$arg.mk' ".
                     "for directory '$dir/$d'\n"
                unless !@r2 or grep { $_ eq $d } @r2;
            }
            for my $d (@r3) {
                $bad = check($d, glob "$dir/$d/*.h") || $bad;
            }
        }
    } else {
        warn "Cannot find info for uor '$arg'.\n";
    }
}
exit($bad);
