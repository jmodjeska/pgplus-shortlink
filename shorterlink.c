/*
 * Playground+ - shorterlink.c
 * Short link for PG+
 * (c) 2018 by Raindog (Jeremy Modjeska)
 * Updated 2018.03.29
 * https://github.com/jmodjeska/pgplus_shortlink/
 * ---------------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>

#ifdef USING_DMALLOC
#include <dmalloc.h>
#endif

#include "include/config.h"
#include "include/player.h"
#include "include/fix.h"
#include "include/proto.h"

/* Function to remove newlines.
   Arguably easier than rewriting the talker in perl */

void chomp(char *s)
{
  s[strcspn ( s, "\n" )] = '\0';
}

/* Function to create the version info (called by version.c) */

void shorterlink_version(void)
{
  ENTERFUNCTION;
  sprintf(stack, " -=> Shorter Links v0.1 (by Raindog) installed.\n");
  stack = strchr(stack, 0);
  EXITFUNCTION;
}

/* Function to generate the link (called my mlink) */

void shorter_link(player * p, char *str)
{
  char *spacer = (char *) NULL;
  char *oldstack = stack;

  ENTERFUNCTION;

  /* vars get an sl_ prefix because I have no idea what might
     conflict with other PG+ global stuff */

  int sl_num = 0;
  int sl_cnt = 0;
  char sl_link[32];
  char sl_conf[256];
  char sl_pre[8];
  char sl_first[79];
  char sl_substr[17];
  char *sl_last = "0";
  char *sl_na;

  /* valid URL protocols by length */
  char sl_pclh[10];   // will hold 4-length (i.e., http://)
  char sl_pclf[10];   // will hold 3-length (i.e., ftp://)
  char sl_pclt[10];   // will hold 6-length (i.e., telnet://)
  char sl_pcls[10];   // will hold 5-length (i.e., https://)

  /* vars for fake regex stuff */
  long sl_upart = 0;
  long sl_unext = 0;
  char sl_line[512];

  /* active prefix text must appear on the fist line of the
     log file exactly as defined: __x where 'x' is any letter */

  char sl_apre[17] = "# Active Prefix:\0";

  /* Check for the links log file;
     If no log file found; inform command unavailable; exit */

  FILE *fp = fopen("logs/links.log","r");
  if (!fp)
  {
    tell_player(p,
        " Sorry, can't find the links database (error logged).\n");
    log("error", "Couldn't find links.log file.");
    EXITFUNCTION;
    return;
  }

  /* Make sure the links log file has a prefix specified on
     the first line that matches for format in sl_apre  */

  sl_first[79]='\0'; /* read the first line */
  fgets(sl_first, 80, fp);

  strncpy(sl_substr, sl_first, 16); /* get the first 16 chars */
  sl_substr[16]='\0';

  if (strcmp(sl_substr, sl_apre) != 0) /* compare against sl_apre */
  {
    tell_player(p,
       " Sorry, error reading the links database (error logged).\n");
    log("error", "Couldn't find valid prefix in links.log.");
    fclose(fp);
    EXITFUNCTION;
    return;
  }

  /* Validate user input */

  if (!*str)
  {
    tell_player(p, " Format: mlink <very long url>\n");
    fclose(fp);
    EXITFUNCTION;
    return;
  }

  /* Look for a reasonable prefix */

  strncpy(sl_pclh, str, 7);
  sl_pclh[7] = '\0';

  strncpy(sl_pclf, str, 6);
  sl_pclf[6] = '\0';

  strncpy(sl_pcls, str, 8);
  sl_pcls[8] = '\0';

  strncpy(sl_pclt, str, 9);
  sl_pclt[9] = '\0';

  if  (   ( strcasecmp("http://", sl_pclh) == 0 ) ||
          ( strcasecmp("ftp://", sl_pclf) == 0 ) ||
          ( strcasecmp("https://", sl_pcls) == 0 ) ||
          ( strcasecmp("telnet://", sl_pclt) == 0 ) )
  {
    // link is OK; do nothing
  }
  else {
    tell_player(p, " This is not a valid URL.\n");
    fclose(fp);
    EXITFUNCTION;
    return;
  }

  /* Look for forbidden characters.
     Should be a faster way to do this ... */

  sl_na = strchr(str,' ');
  while (sl_na!=NULL)
  {
    tell_player(p, " URL cannot contain spaces!\n");
    fclose(fp);
    EXITFUNCTION;
    return;
  }

 /* Allowing semicolons for now, until it results in disaster ...

  sl_na = strchr(str,';');
  while (sl_na!=NULL)
  {
    tell_player(p, " URL cannot contain semicolons!\n");
    fclose(fp);
    EXITFUNCTION;
    return;
  }

 */

  sl_na = strchr(str,'^');
  while (sl_na!=NULL)
  {
    tell_player(p, " URL cannot contain control codes!\n");
    fclose(fp);
    EXITFUNCTION;
    return;
  }
  sl_na = strchr(str,'>');
  while (sl_na!=NULL)
  {
    tell_player(p, " URL cannot contain '>'!\n");
    fclose(fp);
    EXITFUNCTION;
    return;
  }

  /* Check length of URL */

  if (strlen(str) < 16)
  {
    tell_player(p, " That isn't a long URL!\n");
    fclose(fp);
    EXITFUNCTION;
    return;
  }
  if (strlen(str) > 512)
  {
    tell_player(p, " Sorry, that link is too long.\n");
    fclose(fp);
    EXITFUNCTION;
    return;
  }
  if ((spacer = strchr(str, ' ')))
    *spacer = '\0';

  if (spacer)
    *spacer = ' ';

  /* Log file found, input OK: open log, read prefix into sl_pre */

  strncpy(sl_pre, &sl_first[19], 8);
  chomp(sl_pre);

  /* Double-check that we have a valid prefix */

  if ( *sl_pre == '\0' )
  {
    tell_player(p,
       " Sorry, error reading the links database (error logged).\n");
    log("error", "Prefix in links.log is not valid.");
    fclose(fp);
    EXITFUNCTION;
    return;
  }

  /* Find the last line of the file */

  while (fgets(sl_line, sizeof sl_line, fp) != NULL)
  {
    if ( (strchr(sl_line,'>')) != NULL )
    {
      sl_upart = sl_unext;
      sl_unext = ftell(fp);
      sl_cnt++;
    }
  }

  /* If sl_unext is 0 then there was no previous line and
     we're going to start the URL count over. Otherwise there
     was a valid previous entry and we're going to read it */

  /* If our line count is only 1, we don't need to search anymore */

  if ( sl_cnt == 1 )
  {
    sl_last = strchr(sl_line, '>') + 1;
  }

  /* Otherwise keep looking for the last line */

  else if ( sl_unext !=0 && sl_cnt > 0 )
  {
    /* find the last line with a ">" in it and save it */

    fseek(fp, sl_upart, SEEK_SET);
    fgets(sl_line, sizeof sl_line, fp);

    /* find the link component only (after >) */

    sl_last = strchr(sl_line, '>') + 1;
  }

  /* Close the links log */

  fclose(fp);

  /* Extract the number only from sl_last, store in sl_num */

  int i;

  for ( i = 0; i<strlen(sl_last); i++)
  {
    if ( ! isdigit(sl_last[i]) )
    {
      sl_last[i] = ' ';
    }
  }

  sl_num = atoi(sl_last);

  /* Increment sl_num; this is now our new link number */

  sl_num++;

  /* Combine the prefix with the new link number into sl_link */

  sprintf(sl_link, "%s%d", sl_pre, sl_num);

  /* Tell player what their link is */

  pstack_mid(p, "You have added the following link: ");
  stack += sprintf(stack, "\n %s\n", str);
  stack += sprintf(stack,
       "\n ^RShort link to this URL is: http://talker.com/url/%s.^N\n", sl_link);
  stack += sprintf(stack,
       " To share this link with the room, type 'link %s'.\n", sl_link);
  stack += sprintf(stack,
       " To share on a multi or channel, use clink, rlink, etc.");
  sprintf(sl_conf, " ");
  pstack_bot(p, sl_conf);
  stack = end_string(stack);
  tell_player(p, oldstack);
  stack = oldstack;

  /* Write link to logfile (user : url => link name) */

  LOGF("links", "%s: %s => %s", p->name, str, sl_link);

  EXITFUNCTION;
}
