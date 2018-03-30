#!/usr/bin/perl
#
# Short link web forwarding
# (c) 2018 by (Jeremy Modjeska)
# Updated 2018.03.29
# https://github.com/jmodjeska/pgplus_shortlink/
#

use CGI::Carp qw(fatalsToBrowser);
use strict;
use feature 'switch';

# ========================================================================
# Globals, Dependencies
# ========================================================================
my ( $in, $out, $user, $meta, $date );
my @recent;  # array to contain recent links
my @errors;  # array to contain error list
my $nf = 0;  # output page identifier
my $rl = 0;  # recent links flag

# Path to log files containing the links
my $activelog = '/home/talker/logs/links.log';
my $backuplog = '/home/talker/logs/linkslog.txt';

# Path to HTML template
my $link_template = 'link_template.html';

# Number of links to list on the "recent links page"
my $recent_count = 10;

# ========================================================================
# Input
# ========================================================================

# Get input from token; strip non-alphanumeric chars
$in = $ENV{'QUERY_STRING'};
$in =~ s/[\W_]+//g;
if ( $in eq "recent" ) { $rl = 1 }

# ========================================================================
# Process
# ========================================================================

# Error collector
sub error {
  my $e = shift;
  $e = "<p class = \"error\">Whoops, an error occurred :: " . $e . "</p> \n";
  push(@errors, $e);
}

# Find a link in the log file
sub dig {
  if ( ! -e $activelog ) {
    error("Couldn't find active log: $!");
  }
  elsif ( ! -e $backuplog ) {
    error("Couldn't find backup log: $!");
  }
  else {
    my $line;
    my $fnd = 0;
    my @haystack;
    my @otherhaystack;
    my $search = "=> " . $in;
    my $i = 0;

    # Read the log contents into array and close the log
    open(LOG, $activelog) || error("Couldn't read log: $!");
      @haystack = <LOG>;
    close(LOG);

    # Read the log contents into array and close the log
    open(LOG, $backuplog) || error("Couldn't read log: $!");
      @otherhaystack = <LOG>;
    close(LOG);

    @haystack = reverse(@haystack);
    @otherhaystack = reverse(@otherhaystack);
    @haystack = (@haystack, @otherhaystack);

    # Loop through lines in log, look for link pointer
    foreach (@haystack) {
      $line = $_;
      # Look for match if link
      if ( ( $rl == 0 ) && ($line =~ m/$search\n/ ) ) {
        $fnd = 1;
        last;
      }
      # Otherwise add line links to the recent links array
      elsif ( ( $rl == 1 ) && ( $i < $recent_count ) ) {
        push(@recent, $line);
        $i++;
      }
    }

    # Case if no match found and we're looking for a link
    if ( ( ! $fnd == 1 ) && ( $rl == 0 ) ) {
      # Set the output page type
      $nf = 1;
    }

    # Case if we want the recent links list
    elsif ( $rl == 1 ) {
      # Set the output page type
      $nf = 3;
      my ( $rlink, $rdate, $ruser, $rout, $rtrunc );

      foreach my $r ( @recent ) {
        # Extract the date, username, and URL from each line
        # Lines in the log file take the format:
        # hh:mm:ss - mm/dd/yyyy - USER: URL => LINK_NAME
        $r      =~ m/\- (.*) \- (.*)\: (.*) \=\> (.*)\n/g;
        $rdate  = $1;
        $ruser  = $2;
        $rout   = $3;
        $rlink  = $4;
        if ( length( $rout ) > 40 ) {
          $rtrunc = substr( $rout, 0, 40 ) . " ...";
        }
        else { $rtrunc = $rout }

        $r = "<td width = \"110px\">$rdate</td>\n " .
             "<td width = \"200px\">$ruser</td>\n " .
             "<td width = \"450px\"><a href = \"$rout\">$rtrunc</a></td>\n" .
             "<td width = \"40px\"><a href = \"$rout\">$rlink</a></td>\n";
      }
    }

    # Case if match found or we're outputting the recent links page
    else {
      # Set output page type
      $nf = 2;

      # Extract the date, username, and URL from the matching line
      # Lines in the log file take the format:
      # hh:mm:ss - mm/dd/yyyy - USER: URL => LINK_NAME
      $line =~ m/\- (.*) \- (.*)\: (.*) \=\>/g;
      $date = $1;
      $user = $2;
      $out  = $3;
    }
  }
}

# ========================================================================
# HTML Assembly
# ========================================================================

sub html_table {
  my ( $rcv, $go, $user, $date ) = @_;

  # Create a new var to hold the link name; this one will
  # get line breaks every 50 chars so is displays nicely
  # in the table.
  my $pretty = $go;
  $pretty =~ s/\S{50}/$&<br>/sg;

  # Output the forwarding information
  return "<!-- link created by $user on $date -->
      <p><b><a href = \"$go\">$pretty</a></b></p>
      <p>Link created by: <b>$user</b></p>
  \n";
}

sub html_recent {
  # Output the forwarding information
  my $recent_table = "<!-- recent links table -->
      <table class = \"links\" width = \"800px\">
        <tr>
          <td width = \"110x\"><b>Date Added</b></td>
          <td width = \"200px\"><b>User</b></td>
          <td width = \"450px\"><b>URL</b></td>
          <td width = \"40px\"><b>UberLink</b></td>
        </tr>
  \n";

  foreach my $r (@recent) {
    if ( length ( $r ) > 3 ) {
      $recent_table = $recent_table . "<tr>\n$r</tr>\n";
    }
  }
  $recent_table = $recent_table . "\n</table>\n";

  return $recent_table;
}

sub html_text {
  my $txt = shift;
  return "\n<p class = \"default\">$txt</p>\n";
}

sub merge_html {
  my ( $heading, $content, $meta ) = @_;
  my $template;

  open(my $fh, '<', $link_template) or die "cannot open file $link_template";
    {
      local $/;
      $template = <$fh>;
    }
  close($fh);

  # Generate a meta tag if this is a forwarding page
  if ( $meta ) {
    $meta = "<meta http-equiv = \"refresh\" content = \"1; URL=$meta\">";
    $template =~ s/<!-- META -->/$meta/g;
  }

  # Replace variable data elements in the template
  $template =~ s/<!-- HEADING -->/$heading/g;
  $template =~ s/<!-- CONTENT -->/$content/g;

  print $template;
}

# ========================================================================
# Output
# ========================================================================

print "Content-type: text/html\n\n";

# Run the search function if we have a valid search
unless ( length($in) < 1 ) { &dig }

# Set page-type flag if any errors
if ( @errors > 0 ) { $nf = 4 }

# Determine what page to produce
given ($nf) {
  when(1) {
    # Link not found in log file
    my $content = "Whoops, the requested link was not found in the database. ";
    &merge_html("Not Found", $content);
  }
  when(2) {
    # Link found; forward to new page
    my $content = &html_table($in,$out,$user,$date);
    &merge_html("Forwarding ...", $content, $out);
  }
  when(3) {
    # Recent links page
    my $header = "Last $recent_count Links Added";
    my $content = &html_recent;
    &merge_html($header, $content);
  }
  when(4) {
    # Error page
    my $content;
    foreach ( @errors ) {
      $content = $content . $_;
    }
    &merge_html("Not Found", $content);
  }
	default {
    my $content = "This is an automatic URL forwarding service.";
    &merge_html("URL Forwarding", $content);
  }
}
