#!/usr/bin/perl
#
# Short link log rotate script
# Call with cron daily
# (c) 2018 by Raindog (Jeremy Modjeska)
# Updated 2018.03.29
# https://github.com/jmodjeska/pgplus_shortlink/
#

use strict;

#
## Log Locations
#

my $alog = '/home/uber/talker/logs/links.log';
my $mlog = '/home/uber/talker/logs/linkslog.txt';
my $prex = "\# Active Prefix: __";

# Globals
my ( $pos, $char, $stamp, $last, $test, $line );
my $aprx = "z";
my $line = "WIBBLE: bad log sync on " . $stamp . " (get last line) \n";
my $copy = "WIBBLE: bad log sync on " . $stamp . " (copy to master) \n";

# Generate timestamp
my ( $sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst ) =
   localtime(time);

$stamp = sprintf "%4d-%02d-%02d %02d:%02d:%02d\n",
         $year+1900,$mon+1,$mday,$hour,$min,$sec;

#
## Read data from the active log
#

open ( ALOG, $alog ) or die "Can't open $alog for reading: $!\n";
  $pos = -2;  # get past EOF and last newline
    while( $char ne "\n" ) {
      seek ALOG, $pos, 2;
      read ALOG, $char, 1;
      $pos--;
  }
  $line = <ALOG>;
  close ( ALOG );

# Extract active prefix and most recent link number from last line
if ( $line =~ /\=\>\s(\D)(.*)\n/ ) {
  $aprx = $1;
  $last = $2;
}
else {
  die ( "Couldn't find a good prefix in the active log ($alog) \n" );
}

$prex = $prex . $aprx;

#
## Append to master log
#

# Init total line counter
my $linz = 0;

# Append all contents from active log except first two lines,
# last line, and blank lines. Save last line in $test for next step
open ( MLOG, ">>$mlog" ) or die "Can't open $mlog for appending: $! \n";
  open ( ALOG, "<$alog" ) or die "Can't open $alog for reading: $! \n";
    while ( <ALOG> ) {
      unless ( substr($_,0,1) eq "#"
        or $_ eq $line
        or $_ eq "\n"
      ) {
      print MLOG ("$_");
      $test = $_;
      $linz++;
    }
  }
  close ( ALOG );
close ( MLOG );

# Read master log contents into array
open ( MLOG, $mlog ) or die "Can't open $mlog for reading: $! \n";
    my @arr = <MLOG>;
close ( MLOG );

# Confirm move by looking for test line
my $match = 0;

foreach my $ln ( @arr ) {
  if ( $ln eq $test ) {
    $match++;
    last;
  }
}

if ( $linz > 0 && $match < 1 ) {
  die ( "Failed validation of active ($alog) " .
        "to master ($mlog). $match matches / $linz lines. \n" );
}

#
## Wipe the active log
#

# Paste first and last line back in:
# The code requires the log file to specify the current "active
# prefix" on the first line. It then numbers links based on the
# most recent entry (else it starts over at 1), so keeping the
# last line allows numbering to continue without changing the "active
# prefix". If the numbering gets out of hand, increment the "active
# prefix" by one letter (i.e., # Active Prefix: __x).

open ( ALOG, ">$alog" ) or die "Can't open $alog for overwrite: $!\n";
  print ALOG "$prex\n\n";
  print ALOG $line;
close ( ALOG );

#
### Debug
#

# print "Prefix is: $prex \n";
# print "Current number is: $last \n";
# print "Line is: $line \n";
# print "Match count is $match \n"
