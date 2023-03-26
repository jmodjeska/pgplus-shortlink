# Short Link for PG+
A link generator for Playground Plus talkers.

## Usage

```
$ mlink http://thisisaverylongurlexample.com/omg=12345

=================== You have added the following link:  ===================

 http://thisisaverylongurlexample.com/omg=12345

 Short link to this URL is: http://talker_url.org/url/e123.
 To share this link with the room, type 'link e123'.
 To share on a multi or channel, use clink, rlink, etc.

===========================================================================
````

## Live Example
This code is running on the [UberWorld](http://uberworld.org) talker, where I created this link: http://uberworld.org/url/e296.

## Requirements

1. Functioning [Playground Plus](https://github.com/talkers/pgplus) talk server.

1. Public-facing website able to [serve and execute CGI scripts](https://www.techrepublic.com/blog/diy-it-guy/diy-enable-cgi-on-your-apache-server/).

## Installation & Configuration: Talker

1. Add `shoterlink.c` to `/src`.

1. Update line 300 of `shorterlink.c` with your talker's URL and the path to your talker's website where you plan to install the short link redirector (I recommend talker.com/url/), e.g.:

    ```
    "\n ^RShort link to this URL is: http://talker.com/url/%s.^N\n", sl_link);
    ```

1. Update `src/version.c` by inserting the following code at ~[line 149](https://github.com/talkers/pgplus/blob/master/src/version.c#L149):

    ```

    FILE *fp = fopen("src/shorterlink.c","r");
    if (fp)
    {
      shorterlink_version();
      fclose(fp);
    }

    ```

1. Update `src/include/clist.h` by inserting the following code at ~[line 165](https://github.com/talkers/pgplus/blob/master/src/include/clist.h#L165):

    ```

    /* short link generator */
    extern command_func    shorter_link;

    ```

1. Update `src/include/clist.h` by inserting the following code at ~[line 868](https://github.com/talkers/pgplus/blob/master/src/include/clist.h#L868):

    ```

    {"mlink", shorter_link, 0, 0, 1, 0, MISCc},

    ```

1. Append the following code at the end of `doc/help`:

    ```

    :mlink

     ^ZCommand  :^N mlink <long url>

     ^ZDetails  :^N Creates a shorter link through the talker. Useful
                for sharing long URLs on the talker without causing line
                wraps.

     ^ZNotes    :^N * URLs are limited to 500 characters
                * After you create a link, you can share it with the room,
                  a channel, or a multi using the 'link' social

     ^ZSee also :^N link (social)
    ```

1. Create a social called `link` to allow people to easily share their links. If you store socials in `files/socials`, you can use the example provided. Otherwise, do it manually.

1. Add the `logs` directory at the PG+ root level (same level as `src`), including the two log files provided. Ensure permissions on both log files are `644`:

    ```
    chmod 644 links.log
    chmod 644 linkslog.txt
    ```

1. Recompile using the standard PG+ compilation process.

## Installation & Configuration: Website

1. Drop the `url/` directory and its contents into your website's root directory. Ensure proper permissions:

    ```
    chmod 755 url
    chmod 755 logrotate.pl
    chmod 755 link.cgi
    chmod 664 link_template.html
    ```

 1. Update the absolute paths in lines 22 and 23 of `link.cgi` to the location of the two files in the `logs` directory:

    ```
    # Path to log files containing the links
    my $activelog = '/home/talker/logs/links.log';
    my $backuplog = '/home/talker/logs/linkslog.txt';
    ````

1. Customize `link_template.html` to your liking; just don't remove the three required placeholders: `<!-- META -->`, `<!-- HEADING -->`, and `<!-- CONTENT -->`.

## Maintenance
1. Add a cron job to rotate the logs daily. Something like this:

    ```
    55 01 * * * $HOME/website/url/logrotate.pl
    ```

2. Whenever your link numbers get unwieldy, you can change the prefix in `links.log` to a new letter:

    ```
    # Active Prefix: __e
    ```

## You might also like
* [PG+ Test Harness](https://github.com/jmodjeska/pgplus-test)
* [PG+ Cocktail Recipe](https://github.com/jmodjeska/pgplus-cocktail)
