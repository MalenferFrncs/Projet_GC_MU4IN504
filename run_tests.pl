#!/usr/bin/perl

use strict;
use warnings;
use autodie qw(system);
use feature 'say';


# Since tests are ran in alphabetical order; keeping this list sorted
# should help debugging.
#
# Note that in the following list, the results are formed from a
# concatenation of the program's stdout with their result.
my %tests = (
    #    test name               =>    result
    'appterm/facto_tailrec'       => '2432902008176640000',
    'appterm/fun_appterm'         => '1',

    'bench/list_1'                => '10000',
    'bench/list_2'                => '1000000',
    'bench/list_3'                => '100000',
    'bench/list_4'                => '100000',
    'bench/list_5'                => '[1,[5,0]]',
    'bench/list_5'                => '[1,[5,0]]',

    'block_values/array_access'   => '1',
    'block_values/array_set'      => '[0,1,2]',
    'block_values/array_sum'      => '6',
    'block_values/couple'         => '100',
    'block_values/insertion_sort' => '[1,[2,[3,[4,[5,0]]]]]',
    'block_values/liste_iter'     => 'BONJOUR0',
    'block_values/liste_length'   => '3',
    'block_values/liste'          => '[1,[2,[3,[4,0]]]]',
    'block_values/ref'            => '3',

    'exceptions/exn2'             => '88',
    'exceptions/exnexn'           => '23',
    'exceptions/exn'              => '0',
    'exceptions/exn_pop'          => '40',
    'exceptions/exn_uncaught'     => 'Uncaught exception: 0',

    'n-ary_funs/grab1'            => '3',
    'n-ary_funs/grab2'            => '3',
    'n-ary_funs/grab3'            => '21',
    'n-ary_funs/grab4'            => '8',

    'rec_funs/facto'              => '120',
    'rec_funs/fibo'               => '21',

    'unary_funs/arithexpr'        => '10',
    'unary_funs/const'            => '42',
    'unary_funs/fun1'             => '10',
    'unary_funs/fun2'             => '5',
    'unary_funs/fun3'             => 'A0',
    'unary_funs/fun4'             => '42',
    'unary_funs/fun5'             => '41',
    );

# Recompiling the vm
system("cd src && make clean && make");
say "Compilation OK.\n\nRunning tests...\n";


# Running the tests
my ($err_count, $total) = (0, 0);

for my $test (sort keys %tests) {
    my $filename = "tests/$test.txt";
    my $res = `./src/minizam $filename -res 2>&1`;
    chomp $res;
    if ($res ne $tests{$test}) {
        $err_count++;
        say "[error] $test";
        say "  expected: $tests{$test}";
        say "  got: $res";
    } else {
        say "[OK] $test";
    }
    $total++;
}

# Outputting statistics
say "\n\nRan $total tests. $err_count failure" .
    ($err_count > 1 ? "s" : "") . ". " .
    ($total-$err_count) . " success" .
    ($total-$err_count > 1 ? "es" : "") . ".\n";
