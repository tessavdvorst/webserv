#!/usr/bin/env perl

=head1 DESCRIPTION

printenv — a CGI program that just prints its environment

=cut
print "Content-Type: text/plain\r\n\r\n";

foreach ( sort keys %ENV ) {
    print "$_=\"$ENV{$_}\"\r\n";
}
