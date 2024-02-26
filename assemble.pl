#!/usr/bin/perl

=head1 description

WARNING: this script is obsolete.

This script assembles a textual bytecode file of the
minizam. Concretely, it does 2 things:

  - it computes jump addresses

  - it creates an array of primitives, and replaces the instructions
    PRIM <name> with PRIM <index>, where <index> is an integer used to
    index the primitive in the array of pirmitive.

=cut


use strict;
use warnings;
use feature 'say';

my %labels;
my @instrs;
my $index = 0;

# Reading bytecode file
while (<>) {
    next if /^\s*$/;
    next if /^\s*#/; # skip comments
    chomp;
    my %instr = (index => $index);
    if (s/^\s*(\S+)://) {
        $labels{$1} = \%instr;
    }
    $_ = <> while ! /\S/; # in case the label for not on the same line as the instruction
    s/^\s*(\S+)\s*//;
    $instr{instr} = $1;
    my @opts = split /\s*,\s*/;
    $instr{opts} = \@opts;
    push @instrs, \%instr;
    $index += 1 + @opts;
}


my %primitives = (
    '+' => 'ADD',
    '-' => 'SUB',
    '/' => 'DIV',
    '*' => 'MUL',
    'or' => 'OR',
    'and' => 'AND',
    'not' => 'NOT',
    '<>'  => 'NE',
    '='   => 'EQ',
    '<'   => 'LT',
    '<='  => 'LE',
    '>'   => 'GT',
    '>='  => 'GE',
    'print' => 'PRINT');

# Assembling
for my $instr (@instrs) {
    if ($instr->{instr} =~ /^(BRANCH|CLOSURE)/) {
        my $label = $instr->{opts}->[0];
        $instr->{opts}->[0] = $labels{$label}->{index};
    }
    if ($instr->{instr} =~ /^PRIM/) {
        my $prim = $instr->{opts}->[0];
        $instr->{opts}->[0] = $primitives{$prim};
    }
}


# Printing resulting file
printf "code_t* code = { %s };\n", join ", ", map { ($_->{instr}, @{$_->{opts}}) } @instrs;
