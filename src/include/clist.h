/* Add after other command declarations; before multis */
/* short link generator */
extern command_func    shorter_link;

/* Add to the main command list that starts with:
  struct command  complete_list[] = {

  put in the appropriate place based on alphabetical
  command order
*/

{"mlink", shorter_link, 0, 0, 1, 0, MISCc},