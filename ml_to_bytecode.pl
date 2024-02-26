#!/usr/bin/perl

use strict;
use warnings;
use feature qw( say );
use autodie qw( system open close );

use File::Temp qw( tempdir );
use File::Copy;
use File::Basename;
use Cwd;

my $source_file = $ARGV[0] || die "Missing filename";
my ($filename, $dirname) = fileparse($source_file, ".ml");

my $dir = tempdir(CLEANUP => 1);
copy $source_file, $dir;

my $initial_dir = getcwd;
chdir $dir;

# Compiling a first time to detect errors in the .ml. Note that this
# will kill the program in case of an error thanks to autodie pragma.
system("ocamlc $filename.ml");

# Getting the bytecode
my $bytecode = `ocamlc -dinstr $filename.ml 2>&1`;

# Cleaning / formatting the bytecode
my @instrs = qw(
    CONST PRIM BRANCH BRANCHIFNOT PUSH POP ACC
    ENVACC CLOSURE APPLY RETURN STOP
    CLOSUREREC OFFSETCLOSURE
    GRAB RESTART
    APPTERM
    MAKEBLOCK GETFIELD VECTLENGTH GETVECTITEM
    SETFIELD SETVECTITEM ASSIGN
    PUSHTRAP POPTRAP RAISE);


my @bytecode;
for my $line (split /\n/, $bytecode) {
    $line = uc($line);

    $line =~ s/neqint/PRIM <>/i;
    $line =~ s/eqint/PRIM =/i;
    $line =~ s/offsetint (-?\d+)/PUSH\n\tCONST $1\n\tPRIM +/i;
    $line =~ s/ccall caml_lessthan.*/PRIM </i;
    $line =~ s/gtint/PRIM >/i;
    $line =~ s/ltint/PRIM </i;
    $line =~ s/addint/PRIM +/i;
    $line =~ s/mulint/PRIM */i;
    $line =~ s/subint/PRIM -/i;

    $line =~ s/closurerec (\d+)/CLOSUREREC L$1/i;
    $line =~ s/strictbranchifnot/BRANCHIFNOT/i;
    $line =~ s/const 0a/CONST 0/i;
    $line =~ s/ccall caml_array_set_addr.*/SETVECTITEM/i;
    $line =~ s/ccall caml_array_get_addr.*/GETVECTITEM/i;
    $line =~ s/const '(\w)'/"CONST " . ord($1)/ie;

    $line =~ s/branchif(?!not)/PRIM not\n\tBRANCHIFNOT/i;

    $line =~ s/check_signals//i;

    next if $line =~ /setglobal/i;

    if ($line =~ /makeblock/i) {
        if ($line =~ /, 0/) {
            $line =~ s/makeblock (\d+), 0/MAKEBLOCK $1/i;
        } else {
            die "Invalid instruction: $line";
        }
    }

    if ($line =~ /offsetclosure/i) {
        if ($line =~ /offsetclosure 0(?!\d)/i) {
            $line =~ s/ 0//;
        } else {
            die "Invalid instruction: $line";
        }
    }

    if ($line =~ /const \[/i) {
        my @l;
        while ($line =~ /0: (\d+)/g) {
            push @l, $1;
        }
        @l = reverse @l;
        $line = "\tCONST 0" .
            join("", map { "\n\tPUSH\n\tCONST $_\n\tMAKEBLOCK 2" } @l);
    }

    if ($line =~ /pop (\d+)/i) {
        $line = join "\n", ("\tPOP")x$1;
    }


    push @bytecode, $line;
}
push @bytecode, "\tSTOP";

my $fixed_bytecode = join "\n", @bytecode;
$fixed_bytecode =~ s/:\s*\n\s*/:\t/; # Fixing labels to next non-empty line

$fixed_bytecode =~ s/APPLY \d+.*\K\n(.*(ACC|MAKEBLOCK|POP|PUSH).*\n)+.*STOP/\n\tSTOP/;

$fixed_bytecode =~ s/GETGLOBAL PERVASIVES!\n\tGETFIELD 28\n\tPUSH\n\tACC \d+\n\tAPPLY 2/PRIM print/;

say $fixed_bytecode;

# Printing the bytecode to the .txt file
chdir $initial_dir;
open my $FH, '>', "$dirname$filename.txt";
say $FH $fixed_bytecode;
